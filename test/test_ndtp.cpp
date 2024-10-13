#include <gtest/gtest.h>
#include <science/scipp/status.h>
#include <science/synapse/util/ndtp_types.h>

namespace synapse {

TEST(NDTPTest, NDTP_HEADER) {
  NDTPHeader header(DataType::kBroadband, 1234567890, 42);
  auto packed = header.pack();
  auto unpacked = NDTPHeader::unpack(packed);
  EXPECT_TRUE(unpacked == header);



  // invalid version
  auto INVALID_VERSION = 0x02;
  std::vector<uint8_t> invalid_version_data;
  invalid_version_data.push_back(INVALID_VERSION);

  auto data_type = static_cast<std::underlying_type_t<DataType>>(DataType::kBroadband);
  invalid_version_data.insert(invalid_version_data.end(),
                            reinterpret_cast<const uint8_t*>(&data_type),
                            reinterpret_cast<const uint8_t*>(&data_type) + sizeof(DataType));
  uint64_t timestamp = 123;
  invalid_version_data.insert(invalid_version_data.end(),
                            reinterpret_cast<const uint8_t*>(&timestamp),
                            reinterpret_cast<const uint8_t*>(&timestamp) + sizeof(uint64_t));
  uint16_t seq_number = 42;
  invalid_version_data.insert(invalid_version_data.end(),
                            reinterpret_cast<const uint8_t*>(&seq_number),
                            reinterpret_cast<const uint8_t*>(&seq_number) + sizeof(uint16_t));

  EXPECT_THROW(NDTPHeader::unpack(invalid_version_data), std::invalid_argument);


  // insufficient data size
  std::vector<uint8_t> insufficient_data;
  insufficient_data.push_back(0x01);
  insufficient_data.insert(insufficient_data.end(),
                            reinterpret_cast<const uint8_t*>(&data_type),
                            reinterpret_cast<const uint8_t*>(&data_type) + sizeof(DataType));
  insufficient_data.insert(insufficient_data.end(),
                            reinterpret_cast<const uint8_t*>(&timestamp),
                            reinterpret_cast<const uint8_t*>(&timestamp) + sizeof(uint64_t));

  EXPECT_THROW(NDTPHeader::unpack(insufficient_data), std::invalid_argument);

}

TEST(NDTPTest, NDTP_PAYLOAD_ELECTRICAL_BROADBAND_DATA) {
  uint32_t bit_width = 12;
  uint32_t sample_rate = 3;
  bool is_signed = false;
  std::vector<NDTPPayloadBroadbandChannelData> channels;
  channels.push_back(NDTPPayloadBroadbandChannelData(0, {1, 2, 3}));
  channels.push_back(NDTPPayloadBroadbandChannelData(1, {4, 5, 6}));
  channels.push_back(NDTPPayloadBroadbandChannelData(2, {3000, 2000, 1000}));

  NDTPPayloadBroadband payload(is_signed, bit_width, sample_rate, channels);
  auto packed = payload.pack();
  auto unpacked = NDTPPayloadBroadband::unpack(packed);
  EXPECT_EQ(unpacked.bit_width, bit_width);
  EXPECT_EQ(unpacked.is_signed, is_signed);
  EXPECT_EQ(unpacked.sample_rate, sample_rate);
  EXPECT_EQ(unpacked.channels.size(), channels.size());

  EXPECT_EQ(unpacked.channels[0].channel_id, 0);


  EXPECT_EQ(unpacked.channels[1].channel_id, 1);
  EXPECT_EQ(unpacked.channels[1].channel_data, std::vector<float>({4, 5, 6}));

  EXPECT_EQ(unpacked.channels[2].channel_id, 2);
  EXPECT_EQ(unpacked.channels[2].channel_data,
            std::vector<float>({3000, 2000, 1000}));

  EXPECT_EQ(packed[0] >> 1, bit_width);

  EXPECT_EQ((packed[1] << 16) | (packed[2] << 8) | packed[3], sample_rate);
}

TEST(NDTPTest, NDTP_PAYLOAD_SPIKETRAIN_DATA) {
  std::vector<int> spike_counts = {1, 2, 3, 2, 1};
  NDTPPayloadSpiketrain payload(spike_counts);
  auto packed = payload.pack();
  auto unpacked = NDTPPayloadSpiketrain::unpack(packed);
  EXPECT_EQ(unpacked.spike_counts, spike_counts);
}

} // namespace synapse
