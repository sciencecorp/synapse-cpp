#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "science/synapse/api/datatype.pb.h"

namespace synapse {

class NDTPPayloadBroadbandChannelData {
 public:
  NDTPPayloadBroadbandChannelData(int channel_id, const std::vector<float>& channel_data);

  [[nodiscard]] bool operator==(const NDTPPayloadBroadbandChannelData& other) const {
    return channel_id == other.channel_id && channel_data == other.channel_data;
  }
  [[nodiscard]] bool operator!=(const NDTPPayloadBroadbandChannelData& other) const { return !(*this == other); }

  int channel_id;
  std::vector<float> channel_data;
};

class NDTPPayloadBroadband {
 public:
  NDTPPayloadBroadband(
      bool is_signed, size_t bit_width, int sample_rate,
      const std::vector<NDTPPayloadBroadbandChannelData>& channels
  );

  bool is_signed;
  uint32_t bit_width;
  uint16_t sample_rate;
  std::vector<NDTPPayloadBroadbandChannelData> channels;

  auto pack() const -> std::vector<uint8_t>;
  static auto unpack(const std::vector<uint8_t>& data) -> NDTPPayloadBroadband;

  [[nodiscard]] bool operator==(const NDTPPayloadBroadband& other) const {
    return (
        is_signed == other.is_signed && bit_width == other.bit_width && sample_rate == other.sample_rate &&
        channels == other.channels
    );
  }

  [[nodiscard]] bool operator!=(const NDTPPayloadBroadband& other) const { return !(*this == other); }
};

class NDTPPayloadSpiketrain {
 public:
  explicit NDTPPayloadSpiketrain(const std::vector<int>& spike_counts);

  std::vector<int> spike_counts;

  auto pack() const -> std::vector<uint8_t>;
  static auto unpack(const std::vector<uint8_t>& data) -> NDTPPayloadSpiketrain;

  [[nodiscard]] bool operator==(const NDTPPayloadSpiketrain& other) const;
  [[nodiscard]] bool operator!=(const NDTPPayloadSpiketrain& other) const;
};

class NDTPHeader {
 public:
  NDTPHeader(synapse::DataType data_type, uint64_t timestamp, uint32_t seq_number);

  auto pack() const -> std::vector<uint8_t>;
  static auto unpack(const std::vector<uint8_t>& data) -> NDTPHeader;
  auto get_data_type() const -> synapse::DataType { return data_type; }

  [[nodiscard]] bool operator==(const NDTPHeader& other) const {
    return data_type == other.data_type && timestamp == other.timestamp && seq_number == other.seq_number;
  }
  [[nodiscard]] bool operator!=(const NDTPHeader& other) const { return !(*this == other); }

  synapse::DataType data_type;
  uint64_t timestamp;
  uint32_t seq_number;
};

class NDTPMessage {
 public:
  NDTPMessage(const NDTPHeader& header, std::variant<NDTPPayloadBroadband, NDTPPayloadSpiketrain> payload);

  NDTPHeader header;
  std::variant<NDTPPayloadBroadband, NDTPPayloadSpiketrain> payload;

  static auto crc16(const std::vector<uint8_t>& data, uint16_t poly = 0x8005, uint16_t init = 0xFFFF) -> uint16_t;
  static auto crc16_verify(const std::vector<uint8_t>& data, uint16_t crc) -> bool;
  auto pack() const -> std::vector<uint8_t>;
  static auto unpack(const std::vector<uint8_t>& data) -> NDTPMessage;

  bool operator==(const NDTPMessage& other) const;
  bool operator!=(const NDTPMessage& other) const;
};

}  // namespace synapse
