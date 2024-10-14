#include "science/synapse/util/ndtp.h"

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#ifdef __APPLE__
#include <libkern/OSByteOrder.h>

inline uint16_t htole16(uint16_t host_16bits) {
  return OSSwapHostToLittleInt16(host_16bits);
}

inline uint16_t le16toh(uint16_t little_endian_16bits) {
  return OSSwapLittleToHostInt16(little_endian_16bits);
}
#else
#include <endian.h>
#endif

namespace synapse {

template <typename T>
auto to_ints(
    const std::vector<uint8_t>& data, int bit_width, int count, int start_bit, bool is_signed,
    bool byteorder_is_little
) -> std::tuple<std::vector<T>, int, std::vector<uint8_t>> {
  if (bit_width <= 0) {
    throw std::invalid_argument("bit width must be > 0");
  }

  int truncate_bytes = start_bit / 8;
  start_bit = start_bit % 8;

  std::vector<uint8_t> truncated_data(data.begin() + truncate_bytes, data.end());
  size_t data_len = truncated_data.size();

  if (count > 0 && data_len < (bit_width * count + 7) / 8) {
    throw std::invalid_argument(
        "insufficient data for " + std::to_string(count) + " x " + std::to_string(bit_width) +
        " bit values (expected " + std::to_string((bit_width * count + 7) / 8) + " bytes, given " +
        std::to_string(data_len) + " bytes)"
    );
  }

  int current_value = 0;
  int bits_in_current_value = 0;
  int mask = (1 << bit_width) - 1;
  int total_bits_read = 0;
  int value_index = 0;
  int max_values = count > 0 ? count : (data_len * 8) / bit_width;
  std::vector<T> values_array(max_values);
  int bit_width_minus1 = bit_width - 1;
  int sign_bit = 1 << bit_width_minus1;
  uint8_t byte;
  for (size_t byte_index = 0; byte_index < data_len; ++byte_index) {
    byte = truncated_data[byte_index];

    if (byteorder_is_little) {
      int start = byte_index == 0 ? start_bit : 0;
      for (int bit_index = start; bit_index < 8; ++bit_index) {
        int bit = (byte >> bit_index) & 1;
        current_value |= bit << bits_in_current_value;
        ++bits_in_current_value;
        ++total_bits_read;

        if (bits_in_current_value == bit_width) {
          if (is_signed && (current_value & sign_bit)) {
            current_value = current_value - (1 << bit_width);
          } else {
            current_value = current_value & mask;
          }
          values_array[value_index++] = current_value;
          current_value = 0;
          bits_in_current_value = 0;

          if (count > 0 && value_index == count) {
            int end_bit = start_bit + total_bits_read;
            return std::make_tuple(
                std::vector<T>(values_array.begin(), values_array.begin() + value_index), end_bit, truncated_data
            );
          }
        }
      }
    } else {  // big endian
      int start = byte_index == 0 ? start_bit : 0;
      for (int bit_index = 7 - start; bit_index >= 0; --bit_index) {
        int bit = (byte >> bit_index) & 1;
        current_value = (current_value << 1) | bit;
        ++bits_in_current_value;
        ++total_bits_read;

        if (bits_in_current_value == bit_width) {
          if (is_signed && (current_value & sign_bit)) {
            current_value = current_value - (1 << bit_width);
          } else {
            current_value = current_value & mask;
          }
          values_array[value_index++] = current_value;
          current_value = 0;
          bits_in_current_value = 0;

          if (count > 0 && value_index == count) {
            int end_bit = start_bit + total_bits_read;
            return std::make_tuple(
                std::vector<T>(values_array.begin(), values_array.begin() + value_index), end_bit, truncated_data
            );
          }
        }
      }
    }
  }

  if (bits_in_current_value > 0) {
    if (bits_in_current_value == bit_width) {
      if (is_signed && (current_value & sign_bit)) {
        current_value = current_value - (1 << bit_width);
      } else {
        current_value = current_value & mask;
      }
      values_array[value_index++] = current_value;
    } else if (count == 0) {
      throw std::invalid_argument(
          std::to_string(bits_in_current_value) + " bits left over, not enough to form a complete value of bit width " +
          std::to_string(bit_width)
      );
    }
  }

  if (count > 0) {
    value_index = std::min(value_index, count);
  }

  int end_bit = start_bit + total_bits_read;
  return std::make_tuple(
      std::vector<T>(values_array.begin(), values_array.begin() + value_index), end_bit, truncated_data
  );
}

template <typename T>
auto to_bytes(
    const std::vector<T>& values, int bit_width, std::vector<uint8_t> existing, int writing_bit_offset, bool is_signed,
    bool byteorder_is_little
) -> std::tuple<std::vector<uint8_t>, int> {
  int num_bits_to_write = values.size() * bit_width;
  int bit_offset = existing.empty() ? 0 : (existing.size() - 1) * 8 + writing_bit_offset;
  int total_bits_needed = bit_offset + num_bits_to_write;
  int total_bytes_needed = (total_bits_needed + 7) / 8;

  std::vector<uint8_t> result(total_bytes_needed, 0);
  if (!existing.empty()) {
    std::copy(existing.begin(), existing.end(), result.begin());
  }

  auto min_value = is_signed ? -(static_cast<int64_t>(1) << (bit_width - 1)) : 0;
  auto max_value = is_signed ? ((static_cast<int64_t>(1) << (bit_width - 1)) - 1) : ((static_cast<int64_t>(1) << bit_width) - 1);

  for (const auto& value : values) {
    if (value < min_value || value > max_value) {
      throw std::invalid_argument(
          "Value " + std::to_string(value) + " cannot be represented in " + std::to_string(bit_width) + " bits"
      );
    }

    uint64_t value_unsigned = is_signed && value < 0 ? (static_cast<uint64_t>(1) << bit_width) + value : static_cast<uint64_t>(value);

    for (int bits_remaining = bit_width; bits_remaining > 0;) {
      int byte_index = bit_offset / 8;
      int bit_index = bit_offset % 8;
      int bits_in_current_byte = std::min(8 - bit_index, bits_remaining);
      int shift = bits_remaining - bits_in_current_byte;

      uint8_t bits_to_write = (value_unsigned >> shift) & ((1 << bits_in_current_byte) - 1);
      bits_to_write <<= byteorder_is_little ? bit_index : (8 - bit_index - bits_in_current_byte);

      result[byte_index] |= bits_to_write;
      bits_remaining -= bits_in_current_byte;
      bit_offset += bits_in_current_byte;
    }
  }

  int final_bit_offset = bit_offset % 8;
  if (final_bit_offset == 0 && total_bytes_needed < static_cast<int>(result.size())) {
    result.resize(total_bytes_needed);
  }

  return std::make_tuple(result, final_bit_offset);
}

/**
 * @brief Represents the header of an NDTP (Neural Data Transfer Protocol)
 * message.
 *
 * The NDTPHeader contains metadata about the NDTP message, including:
 * - The version of the NDTP protocol
 * - The synapse::DataType of the payload
 * - A timestamp
 * - A sequence number
 *
 * This class provides methods for packing the header into a byte vector
 * and unpacking a byte vector into a header object.
 */
static const uint8_t kNDTPVersion = 0x01;
static const size_t kNDTPHeaderSize = 15;  // 1 + 1 + 8 + 2 + 3 bytes (struct.Struct("<BIQH"))

NDTPHeader::NDTPHeader(synapse::DataType data_type, uint64_t timestamp, uint32_t seq_number)
    : data_type(data_type), timestamp(timestamp), seq_number(seq_number) {}

auto NDTPHeader::pack() const -> std::vector<uint8_t> {
  std::vector<uint8_t> result(kNDTPHeaderSize);
  uint8_t* ptr = result.data();

  *ptr++ = kNDTPVersion;
  *ptr++ = data_type;
  std::memcpy(ptr, &timestamp, sizeof(timestamp));
  ptr += sizeof(timestamp);
  uint16_t seq_number_le = htole16(seq_number);
  std::memcpy(ptr, &seq_number_le, sizeof(seq_number_le));

  return result;
}

auto NDTPHeader::unpack(const std::vector<uint8_t>& data) -> NDTPHeader {
  if (data.size() < kNDTPHeaderSize) {
    throw std::invalid_argument("Invalid header size: expected at least " + std::to_string(kNDTPHeaderSize) + " bytes");
  }
  const uint8_t* ptr = data.data();

  uint8_t version = *ptr++;
  if (version != kNDTPVersion) {
    throw std::invalid_argument(
        "Incompatible version: expected " + std::to_string(kNDTPVersion) + ", got " + std::to_string(version)
    );
  }

  uint8_t data_type = *ptr++;
  int64_t timestamp;
  std::memcpy(&timestamp, ptr, sizeof(timestamp));
  ptr += sizeof(timestamp);
  uint16_t seq_number_le;
  std::memcpy(&seq_number_le, ptr, sizeof(seq_number_le));
  uint16_t seq_number = le16toh(seq_number_le);

  return NDTPHeader(synapse::DataType(data_type), timestamp, seq_number);
}

/**
 * @brief Represents channel data for broadband NDTP payload.
 *
 * This class encapsulates the data for a single channel in a broadband NDTP payload.
 * It contains the channel ID and the corresponding channel data. The sample count is inferred later and added to each
 * channel payload by the NDTPPayloadBroadband::pack method.
 *
 * The payload consists of:
 * - A 3-byte unsigned integer representing the channel ID
 * - A vector of bit_width signed integers representing the channel data (bit_width is set NDTPPayloadBroadband)
 */
NDTPPayloadBroadbandChannelData::NDTPPayloadBroadbandChannelData(
    int channel_id, const std::vector<float>& channel_data
)
    : channel_id(channel_id), channel_data(channel_data) {}

/**
 * @brief Represents a broadband NDTP payload.
 *
 * This class encapsulates the data for a broadband NDTP payload, including metadata
 * about the signal (such as whether it's signed, its bit width, and sample rate)
 * and the actual channel data.
 *
 * The payload consists of:
 * - A flag indicating whether the data is signed or unsigned
 * - The bit width of each sample
 * - The sample rate of the data
 * - A vector of NDTPPayloadBroadbandChannelData objects, each representing a single channel's data
 */

NDTPPayloadBroadband::NDTPPayloadBroadband(
    bool is_signed, size_t bit_width, int sample_rate,
    const std::vector<NDTPPayloadBroadbandChannelData>& channels
)
    : is_signed(is_signed), bit_width(bit_width), sample_rate(sample_rate), channels(channels) {}

auto NDTPPayloadBroadband::pack() const -> std::vector<uint8_t> {
  std::vector<uint8_t> payload;
  payload.reserve(6 + channels.size() * 5);  // Reserve space for metadata and channel headers
  uint32_t n_channels = channels.size();

  // First byte: bit width and signed flag
  payload.push_back(((bit_width & 0x7F) << 1) | (is_signed ? 1 : 0));

  // Next three bytes: number of channels (24-bit integer)
  payload.push_back((n_channels >> 16) & 0xFF);
  payload.push_back((n_channels >> 8) & 0xFF);
  payload.push_back(n_channels & 0xFF);

  // Next two bytes: sample rate (16-bit integer)
  payload.push_back((sample_rate >> 8) & 0xFF);
  payload.push_back(sample_rate & 0xFF);

  for (const auto& c : channels) {
    // Pack channel_id (3 bytes, 24 bits)
    payload.push_back((c.channel_id >> 16) & 0xFF);
    payload.push_back((c.channel_id >> 8) & 0xFF);
    payload.push_back(c.channel_id & 0xFF);

    // Pack number of samples (2 bytes, 16 bits)
    uint16_t num_samples = c.channel_data.size();
    payload.push_back((num_samples >> 8) & 0xFF);
    payload.push_back(num_samples & 0xFF);

    // Pack channel_data
    auto [channel_data_bytes, final_bit_offset] = to_bytes<float>(c.channel_data, bit_width, {}, 0, is_signed);
    payload.insert(payload.end(), channel_data_bytes.begin(), channel_data_bytes.end());
  }

  return payload;
}

auto NDTPPayloadBroadband::unpack(const std::vector<uint8_t>& data) -> NDTPPayloadBroadband {
  if (data.size() < 6) {
    throw std::runtime_error("Invalid data size for NDTPPayloadBroadband");
  }
  uint32_t bit_width = data[0] >> 1;
  bool is_signed = (data[0] & 1) == 1;
  uint32_t num_channels = (data[1] << 16) | (data[2] << 8) | data[3];
  uint32_t sample_rate = (data[4] << 8) | data[5];

  std::vector<NDTPPayloadBroadbandChannelData> channels;
  size_t offset = 6;
  for (uint32_t i = 0; i < num_channels; ++i) {
    if (offset + 3 > data.size()) {
      throw std::runtime_error("Incomplete data for channel_id");
    }
    // unpack channel_id (3 bytes, big-endian)
    uint32_t channel_id = (data[offset] << 16) | (data[offset + 1] << 8) | data[offset + 2];
    offset += 3;

    // unpack num_samples (2 bytes, big-endian)
    uint16_t num_samples = (data[offset] << 8) | data[offset + 1];
    offset += 2;

    // calculate the number of bytes needed for the channel data
    uint32_t num_bytes = (num_samples * bit_width + 7) / 8;

    // ensure we have enough data
    if (offset + num_bytes > data.size()) {
      throw std::runtime_error("Incomplete data for channel_data");
    }
    std::vector<uint8_t> channel_data_bytes(data.begin() + offset, data.begin() + offset + num_bytes);
    offset += num_bytes;

    // unpack channel_data
    auto [channel_data, end_bit, truncated_data] = to_ints<float>(channel_data_bytes, bit_width, num_samples, 0, is_signed, false);
    channels.emplace_back(channel_id, channel_data);
  }
  return NDTPPayloadBroadband(is_signed, bit_width, sample_rate, channels);
}

/**
 * @brief Constructs an NDTPPayloadSpiketrain object.
 *
 * This class encapsulates a spiketrain payload in the NDTP (Neural Data Transfer Protocol) format.
 * It contains a vector of spike counts for different channels or units.
 *
 * The payload consists of:
 * - A 4-byte unsigned integer representing the number of spike counts
 * - A vector of spike counts, each represented by a 4-byte unsigned integer
 */
static const uint32_t NDTPPayloadSpiketrain_BIT_WIDTH = 2;

NDTPPayloadSpiketrain::NDTPPayloadSpiketrain(const std::vector<int>& spike_counts) : spike_counts(spike_counts) {}

auto NDTPPayloadSpiketrain::pack() const -> std::vector<uint8_t> {
  int num_counts = spike_counts.size();
  int clamp_value = (1 << NDTPPayloadSpiketrain_BIT_WIDTH) - 1;
  std::vector<int> clamped_counts;

  // clamp spike counts to max value allowed by bit width
  for (const int& count : spike_counts) {
    clamped_counts.push_back(std::min(count, clamp_value));
  }

  std::vector<uint8_t> result(4);  // Start with 4 bytes for num_counts
  // pack sample count (4 bytes, little-endian)
  result[0] = num_counts & 0xFF;
  result[1] = (num_counts >> 8) & 0xFF;
  result[2] = (num_counts >> 16) & 0xFF;
  result[3] = (num_counts >> 24) & 0xFF;

  // pack clamped spike counts
  auto [bytes, final_bit_offset] = to_bytes<int>(clamped_counts, NDTPPayloadSpiketrain_BIT_WIDTH, {}, 0, false);
  result.insert(result.end(), bytes.begin(), bytes.end());

  return result;
}

auto NDTPPayloadSpiketrain::unpack(const std::vector<uint8_t>& data) -> NDTPPayloadSpiketrain {
  if (data.size() < 4) {
    throw std::runtime_error("Invalid data size for NDTPPayloadSpiketrain");
  }
  uint32_t num_counts = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);

  std::vector<uint8_t> payload(data.begin() + 4, data.end());
  auto bits_needed = num_counts * NDTPPayloadSpiketrain_BIT_WIDTH;
  auto bytes_needed = (bits_needed + 7) / 8;
  if (payload.size() < bytes_needed) {
    throw std::runtime_error("Insufficient data for spike_counts");
  }
  payload.resize(bytes_needed);
  std::vector<int> spike_counts;
  std::tie(spike_counts, std::ignore, std::ignore) = to_ints<int>(payload, NDTPPayloadSpiketrain_BIT_WIDTH, num_counts, 0, false);
  return NDTPPayloadSpiketrain(spike_counts);
}

/**
 * @brief Represents an NDTP (Neural Data Transfer Protocol) message.
 *
 * This class encapsulates an NDTP message, including header + payload + crc16.
 * The payload is a shared pointer to either NDTPPayloadBroadband or NDTPPayloadSpiketrain.
 */
NDTPMessage::NDTPMessage(const NDTPHeader& header, std::variant<NDTPPayloadBroadband, NDTPPayloadSpiketrain> payload)
    : header(header), payload(payload) {}

auto NDTPMessage::pack() const -> std::vector<uint8_t> {
  auto header_data = header.pack();
  std::vector<uint8_t> result(header_data);

  if (std::holds_alternative<NDTPPayloadBroadband>(payload)) {
    auto payload_data = std::get<NDTPPayloadBroadband>(payload).pack();
    result.insert(result.end(), payload_data.begin(), payload_data.end());
  } else if (std::holds_alternative<NDTPPayloadSpiketrain>(payload)) {
    auto payload_data = std::get<NDTPPayloadSpiketrain>(payload).pack();
    result.insert(result.end(), payload_data.begin(), payload_data.end());
  } else {
    throw std::runtime_error("Unsupported payload type");
  }

  uint16_t crc = crc16(result);
  result.push_back(static_cast<uint8_t>(crc & 0xFF));
  result.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));

  return result;
}

auto NDTPMessage::unpack(const std::vector<uint8_t>& data) -> NDTPMessage {
  if (data.size() < 16) {
    throw std::runtime_error("Invalid data size for NDTPMessage");
  }
  uint16_t received_crc = (static_cast<uint16_t>(data[data.size() - 1]) << 8) | data[data.size() - 2];
  std::vector<uint8_t> payload_bytes(data.begin(), data.end() - 2);
  if (!crc16_verify(payload_bytes, received_crc)) {
    throw std::runtime_error("CRC verification failed");
  }

  auto header = NDTPHeader::unpack(std::vector<uint8_t>(data.begin(), data.begin() + kNDTPHeaderSize));
  if (header.get_data_type() == synapse::DataType::kBroadband) {
    auto unpacked_payload = NDTPPayloadBroadband::unpack(payload_bytes);
    return NDTPMessage(header, unpacked_payload);
  } else if (header.get_data_type() == synapse::DataType::kSpiketrain) {
    auto unpacked_payload = NDTPPayloadSpiketrain::unpack(payload_bytes);
    return NDTPMessage(header, unpacked_payload);
  }

  throw std::runtime_error("Unsupported data type in NDTP header");
}

auto NDTPMessage::crc16(const std::vector<uint8_t>& data, uint16_t poly, uint16_t init) -> uint16_t {
  uint16_t crc = init;
  for (const auto& byte : data) {
    crc ^= byte << 8;
    for (int i = 0; i < 8; ++i) {
      if (crc & 0x8000) {
        crc = (crc << 1) ^ poly;
      } else {
        crc <<= 1;
      }
      crc &= 0xFFFF;
    }
  }
  return crc & 0xFFFF;
}

auto NDTPMessage::crc16_verify(const std::vector<uint8_t>& data, uint16_t crc) -> bool {
  return crc16(data) == crc;
}

}  // namespace synapse
