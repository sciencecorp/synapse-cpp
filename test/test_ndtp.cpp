#include <gtest/gtest.h>
#include <science/scipp/status.h>
#include <science/synapse/util/ndtp.h>
#include <science/synapse/util/ndtp_types.h>

namespace synapse {

TEST(ToBytes, BasicFunctionality) {
  auto [result1, offset1] = to_bytes<int>({1, 2, 3, 0}, 2);
  EXPECT_EQ(result1, (std::vector<uint8_t>{0x6C}));
  EXPECT_EQ(offset1, 0);

  auto [result2, offset2] = to_bytes<int>({1, 2, 3, 2, 1}, 2);
  EXPECT_EQ(result2, (std::vector<uint8_t>{0x6E, 0x40}));
  EXPECT_EQ(offset2, 2);

  auto [result3, offset3] = to_bytes<int>({7, 5, 3, 1}, 12);
  EXPECT_EQ(result3, (std::vector<uint8_t>{0x00, 0x70, 0x05, 0x00, 0x30, 0x01}));
  EXPECT_EQ(offset3, 0);

  auto [result4, offset4] = to_bytes<int>({-7, -5, -3, -1}, 12, {}, 0, true);
  EXPECT_EQ(result4, (std::vector<uint8_t>{0xFF, 0x9F, 0xFB, 0xFF, 0xDF, 0xFF}));
  EXPECT_EQ(offset4, 0);

  auto [result5, offset5] = to_bytes<int>({7, 5, 3}, 12, {0x01, 0x00}, 4);
  EXPECT_EQ(result5, (std::vector<uint8_t>{0x01, 0x00, 0x07, 0x00, 0x50, 0x03}));
  EXPECT_EQ(offset5, 0);

  auto [result6, offset6] = to_bytes<int>({-7, -5, -3}, 12, {0x01, 0x00}, 4, true);
  EXPECT_EQ(result6, (std::vector<uint8_t>{0x01, 0x0F, 0xF9, 0xFF, 0xBF, 0xFD}));
  EXPECT_EQ(offset6, 0);

  auto [result7, offset7] = to_bytes<int>({7, 5, 3}, 12);
  EXPECT_EQ(result7, (std::vector<uint8_t>{0x00, 0x70, 0x05, 0x00, 0x30}));
  EXPECT_EQ(offset7, 4);

  auto [result8, offset8] = to_bytes<int>({1, 2, 3, 4}, 8);
  EXPECT_EQ(result8, (std::vector<uint8_t>{0x01, 0x02, 0x03, 0x04}));
  EXPECT_EQ(offset8, 0);

  std::vector<uint8_t> result9;
  int offset9;
  std::tie(result9, offset9) = to_bytes<int>({7, 5, 3}, 12);
  EXPECT_EQ(result9, (std::vector<uint8_t>{0x00, 0x70, 0x05, 0x00, 0x30}));
  EXPECT_EQ(result9.size(), 5);
  EXPECT_EQ(offset9, 4);

  auto [result10, offset10] = to_bytes<int>({3, 5, 7}, 12, result9, offset9);
  EXPECT_EQ(result10, (std::vector<uint8_t>{0x00, 0x70, 0x05, 0x00, 0x30, 0x03, 0x00, 0x50, 0x07}));
  EXPECT_EQ(result10.size(), 9);
  EXPECT_EQ(offset10, 0);
}

TEST(ToBytes, ErrorCases) {
  // Test case: 8 doesn't fit in 3 bits
  EXPECT_THROW(to_bytes<int>({8}, 3), std::invalid_argument);

  // Test case: Invalid bit width
  EXPECT_THROW(to_bytes<int>({1, 2, 3, 0}, 0), std::invalid_argument);
}

TEST(ToInts, BasicFunctionality) {
  std::vector<int> res;
  int offset;
  std::vector<uint8_t> remaining;

  std::tie(res, offset, remaining) = to_ints<int>({0x6C}, 2);
  EXPECT_EQ(res, std::vector<int>({1, 2, 3, 0}));
  EXPECT_EQ(offset, 8);

  std::tie(res, offset, remaining) = to_ints<int>({0x6C}, 2, 3);
  EXPECT_EQ(res, std::vector<int>({1, 2, 3}));
  EXPECT_EQ(offset, 6);

  std::tie(res, offset, remaining) = to_ints<int>({0x00, 0x70, 0x05, 0x00, 0x30, 0x01}, 12);
  EXPECT_EQ(res, std::vector<int>({7, 5, 3, 1}));
  EXPECT_EQ(offset, 48);

  std::tie(res, offset, remaining) = to_ints<int>({0x6C}, 2, 3, 2);
  EXPECT_EQ(res, std::vector<int>({2, 3, 0}));
  EXPECT_EQ(offset, 6 + 2);

  std::tie(res, offset, remaining) = to_ints<int>({0x00, 0x07, 0x00, 0x50, 0x03}, 12, 3, 4);
  EXPECT_EQ(res, std::vector<int>({7, 5, 3}));
  EXPECT_EQ(offset, 36 + 4);

  std::tie(res, offset, remaining) = to_ints<int>({0xFF, 0xF9, 0xFF, 0xBF, 0xFD}, 12, 3, 4, true);
  EXPECT_EQ(res, std::vector<int>({-7, -5, -3}));
  EXPECT_EQ(offset, 36 + 4);
}

TEST(ToInts, ByteArrayIteration) {
  std::vector<uint8_t> arry = {0x6E, 0x40};
  std::vector<int> res;
  int offset;

  std::tie(res, offset, arry) = to_ints<int>(arry, 2, 1);
  EXPECT_EQ(res, std::vector<int>({1}));
  EXPECT_EQ(offset, 2);

  std::tie(res, offset, arry) = to_ints<int>(arry, 2, 1, offset);
  EXPECT_EQ(res, std::vector<int>({2}));
  EXPECT_EQ(offset, 4);

  std::tie(res, offset, arry) = to_ints<int>(arry, 2, 1, offset);
  EXPECT_EQ(res, std::vector<int>({3}));
  EXPECT_EQ(offset, 6);

  std::tie(res, offset, arry) = to_ints<int>(arry, 2, 1, offset);
  EXPECT_EQ(res, std::vector<int>({2}));
  EXPECT_EQ(offset, 8);
}

TEST(ToInts, ErrorCases) {
  // Invalid bit width
  EXPECT_THROW(to_ints<int>({0x01}, 0), std::invalid_argument);

  // Incomplete value
  EXPECT_THROW(to_ints<int>({0x01}, 3), std::invalid_argument);

  // Insufficient data
  EXPECT_THROW(to_ints<int>({0x01, 0x02}, 3), std::invalid_argument);
}


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
  invalid_version_data.insert(
      invalid_version_data.end(), reinterpret_cast<const uint8_t*>(&data_type),
      reinterpret_cast<const uint8_t*>(&data_type) + sizeof(DataType)
  );
  uint64_t timestamp = 123;
  invalid_version_data.insert(
      invalid_version_data.end(), reinterpret_cast<const uint8_t*>(&timestamp),
      reinterpret_cast<const uint8_t*>(&timestamp) + sizeof(uint64_t)
  );
  uint16_t seq_number = 42;
  invalid_version_data.insert(
      invalid_version_data.end(), reinterpret_cast<const uint8_t*>(&seq_number),
      reinterpret_cast<const uint8_t*>(&seq_number) + sizeof(uint16_t)
  );

  EXPECT_THROW(NDTPHeader::unpack(invalid_version_data), std::invalid_argument);

  // insufficient data size
  std::vector<uint8_t> insufficient_data;
  insufficient_data.push_back(0x01);
  insufficient_data.insert(
      insufficient_data.end(), reinterpret_cast<const uint8_t*>(&data_type),
      reinterpret_cast<const uint8_t*>(&data_type) + sizeof(DataType)
  );
  insufficient_data.insert(
      insufficient_data.end(), reinterpret_cast<const uint8_t*>(&timestamp),
      reinterpret_cast<const uint8_t*>(&timestamp) + sizeof(uint64_t)
  );

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
  EXPECT_EQ(unpacked.channels[2].channel_data, std::vector<float>({3000, 2000, 1000}));

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

TEST(NDTPTest, NDTP_MESSAGE) {
  NDTPHeader header(DataType::kBroadband, 1234567890, 42);
  NDTPPayloadBroadband payload(false, 12, 3, {NDTPPayloadBroadbandChannelData(0, {1, 2, 3})});
  NDTPMessage message(header, payload);
  auto packed = message.pack();
  auto unpacked = NDTPMessage::unpack(packed);
  // EXPECT_EQ(unpacked.header, header);
  // EXPECT_EQ(std::get<NDTPPayloadBroadband>(unpacked.payload), payload);

  // NDTPPayloadSpiketrain payload2(std::vector<int>{1, 2, 3, 2, 1});
  // NDTPMessage message2(header, payload2);
  // auto packed2 = message2.pack();
  // auto unpacked2 = NDTPMessage::unpack(packed2);
  // EXPECT_EQ(unpacked2.header, header);
  // EXPECT_EQ(std::get<NDTPPayloadSpiketrain>(unpacked2.payload), payload2);
}

}  // namespace synapse
