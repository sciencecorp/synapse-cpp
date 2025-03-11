#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <science/synapse/nodes/stream_out.h>
#include <science/synapse/api/nodes/stream_out.pb.h>
#include <science/libndtp/ndtp.h>

using namespace synapse;
using science::libndtp::NDTP_VERSION;
using science::libndtp::NDTPMessage;
using science::libndtp::NDTPHeader;
using science::libndtp::NDTPPayloadBroadband;
using science::libndtp::NDTPPayloadSpiketrain;
using science::libndtp::BinnedSpiketrainData;
using science::libndtp::ByteArray;
using science::libndtp::ElectricalBroadbandData;
using science::libndtp::SynapseData;

class StreamOutTest : public ::testing::Test {
protected:
  void SetUp() override {
  }
};

TEST_F(StreamOutTest, Constructor) {
  StreamOut stream_out("127.0.0.1", 12345, "test label");
  NodeConfig proto;
  EXPECT_TRUE(stream_out.to_proto(&proto).ok());
  
  EXPECT_TRUE(proto.has_stream_out());
  const auto& config = proto.stream_out();
  EXPECT_TRUE(config.has_udp_unicast());
  EXPECT_EQ(config.label(), "test label");
  EXPECT_EQ(config.udp_unicast().destination_address(), "127.0.0.1");
  EXPECT_EQ(config.udp_unicast().destination_port(), 12345);
}

TEST_F(StreamOutTest, DefaultConstructor) {
  StreamOut stream_out;
  NodeConfig proto;
  EXPECT_TRUE(stream_out.to_proto(&proto).ok());
  
  EXPECT_TRUE(proto.has_stream_out());
  const auto& config = proto.stream_out();
  EXPECT_TRUE(config.has_udp_unicast());
  EXPECT_EQ(config.label(), "");
  EXPECT_EQ(config.udp_unicast().destination_port(), StreamOut::DEFAULT_STREAM_OUT_PORT);
}

TEST_F(StreamOutTest, FromProto) {
  NodeConfig proto;
  auto* stream_out_config = proto.mutable_stream_out();
  auto* udp_config = stream_out_config->mutable_udp_unicast();
  udp_config->set_destination_address("192.168.1.1");
  udp_config->set_destination_port(9999);
  stream_out_config->set_label("test label 123");

  std::shared_ptr<Node> node;
  EXPECT_TRUE(StreamOut::from_proto(proto, &node).ok());
  
  // Convert back to proto to verify
  NodeConfig result_proto;
  EXPECT_TRUE(node->to_proto(&result_proto).ok());
  
  EXPECT_TRUE(result_proto.has_stream_out());
  const auto& result_config = result_proto.stream_out();
  EXPECT_EQ(result_config.label(), "test label 123");
  EXPECT_EQ(result_config.udp_unicast().destination_address(), "192.168.1.1");
  EXPECT_EQ(result_config.udp_unicast().destination_port(), 9999);
}

TEST_F(StreamOutTest, FromProtoDefaults) {
  NodeConfig proto;
  proto.mutable_stream_out();  // Empty config

  std::shared_ptr<Node> node;
  EXPECT_TRUE(StreamOut::from_proto(proto, &node).ok());
  
  NodeConfig result_proto;
  EXPECT_TRUE(node->to_proto(&result_proto).ok());
  
  EXPECT_TRUE(result_proto.has_stream_out());
  const auto& config = result_proto.stream_out();
  EXPECT_EQ(config.udp_unicast().destination_port(), StreamOut::DEFAULT_STREAM_OUT_PORT);
}

TEST_F(StreamOutTest, FromProtoInvalid) {
  NodeConfig proto;
  // Don't set stream_out config
  
  std::shared_ptr<Node> node;
  auto status = StreamOut::from_proto(proto, &node);
  EXPECT_FALSE(status.ok());
  EXPECT_EQ(status.code(), science::StatusCode::kInvalidArgument);
}

TEST_F(StreamOutTest, ReadBroadbandData) {
  StreamOut stream_out("127.0.0.1", 12345, "test");
  auto status = stream_out.init();
  EXPECT_TRUE(status.ok()) << status.message();

  NDTPPayloadBroadband payload{
    .is_signed = true,
    .bit_width = 16,
    .ch_count = 2,
    .sample_rate = 30000,
    .channels = {
      {
        .channel_id = 1,
        .channel_data = {1000, 2000, 3000}
      },
      {
        .channel_id = 2,
        .channel_data = {4000, 5000, 6000}
      }
    }
  };

  NDTPMessage msg{
    .header = {
      .version = NDTP_VERSION,
      .data_type = synapse::DataType::kBroadband,
      .timestamp = 123456789,
      .seq_number = 1
    },
    .payload = payload
  };

  auto bytes = msg.pack();
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  ASSERT_GT(sock, 0);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(12345);
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

  auto rc = sendto(sock, bytes.data(), bytes.size(), 0, 
                   reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
  ASSERT_EQ(rc, bytes.size());

  SynapseData received_data;
  status = stream_out.read(&received_data);
  EXPECT_TRUE(status.ok()) << status.message();
  
  ASSERT_TRUE(std::holds_alternative<ElectricalBroadbandData>(received_data));
  const auto& broadband = std::get<ElectricalBroadbandData>(received_data);
  
  EXPECT_EQ(broadband.sample_rate, 30000);
  ASSERT_EQ(broadband.channels.size(), 2);
  EXPECT_EQ(broadband.channels[0].channel_id, 1);
  EXPECT_EQ(broadband.channels[1].channel_id, 2);
  
  std::vector<int64_t> expected_ch1 = {1000, 2000, 3000};
  std::vector<int64_t> expected_ch2 = {4000, 5000, 6000};
  for (size_t i = 0; i < expected_ch1.size(); i++) {
    EXPECT_EQ(broadband.channels[0].channel_data[i], expected_ch1[i]);
  }
  for (size_t i = 0; i < expected_ch2.size(); i++) {
    EXPECT_EQ(broadband.channels[1].channel_data[i], expected_ch2[i]);
  }

  close(sock);
}

TEST_F(StreamOutTest, ReadSpiketrainData) {
  StreamOut stream_out("127.0.0.1", 12346, "test");
  auto status = stream_out.init();
  EXPECT_TRUE(status.ok()) << status.message();

  NDTPPayloadSpiketrain payload{
    .bin_size_ms = 1,
    .spike_counts = {3, 0, 2, 1}
  };

  NDTPMessage msg{
    .header = {
      .version = NDTP_VERSION,
      .data_type = synapse::DataType::kSpiketrain,
      .timestamp = 123456789,
      .seq_number = 1
    },
    .payload = payload
  };

  auto bytes = msg.pack();
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  ASSERT_GT(sock, 0);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(12346);
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

  auto rc = sendto(sock, bytes.data(), bytes.size(), 0,
                   reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
  ASSERT_EQ(rc, bytes.size());

  SynapseData received_data;
  status = stream_out.read(&received_data);
  EXPECT_TRUE(status.ok()) << status.message();
  
  ASSERT_TRUE(std::holds_alternative<BinnedSpiketrainData>(received_data));
  const auto& spiketrain = std::get<BinnedSpiketrainData>(received_data);
  
  EXPECT_EQ(spiketrain.bin_size_ms, 1);
  EXPECT_EQ(spiketrain.spike_counts, std::vector<uint8_t>({3, 0, 2, 1}));

  close(sock);
}

TEST_F(StreamOutTest, ReadInvalidData) {
  StreamOut stream_out("127.0.0.1", 12347, "test");
  auto status = stream_out.init();
  EXPECT_TRUE(status.ok()) << status.message();

  std::vector<uint8_t> invalid_data = {0x01, 0x02, 0x03};
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  ASSERT_GT(sock, 0);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(12347);
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

  auto rc = sendto(sock, invalid_data.data(), invalid_data.size(), 0,
                   reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
  ASSERT_EQ(rc, invalid_data.size());

  SynapseData received_data;
  status = stream_out.read(&received_data);
  EXPECT_FALSE(status.ok());
  EXPECT_EQ(status.code(), science::StatusCode::kInternal);

  close(sock);
}
