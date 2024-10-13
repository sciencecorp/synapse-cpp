
#include <gtest/gtest.h>
#include <science/scipp/status.h>
#include <science/synapse/util/ndtp_types.h>

namespace synapse {


TEST(NDTPTypesTest, NDTP_PAYLOAD_ELECTRICAL_BROADBAND_DATA) {
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

  // EXPECT_EQ(unpacked.channels[0].channel_id, 0);
  // EXPECT_EQ(unpacked.channels[0].channel_data, std::vector<int16_t>({1, 2, 3}));

  // EXPECT_EQ(unpacked.channels[1].channel_id, 1);
  // EXPECT_EQ(unpacked.channels[1].channel_data, std::vector<int16_t>({4, 5, 6}));

  // EXPECT_EQ(unpacked.channels[2].channel_id, 2);
  // EXPECT_EQ(unpacked.channels[2].channel_data,
  //           std::vector<int16_t>({3000, 2000, 1000}));

  // EXPECT_EQ(packed[0] >> 1, bit_width);

  // EXPECT_EQ((packed[1] << 16) | (packed[2] << 8) | packed[3], sample_rate);

  // TODO(kevinc): move more assertions
}

} // namespace synapse
