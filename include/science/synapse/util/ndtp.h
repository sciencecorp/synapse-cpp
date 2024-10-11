#pragma once

#include <cstdint>
#include <vector>
#include <memory>

#include "science/synapse/api/datatype.pb.h"

namespace synapse {

class NDTPPayloadBroadbandChannelData {
public:
    NDTPPayloadBroadbandChannelData(uint32_t channel_id, const std::vector<int16_t>& channel_data);

    uint32_t channel_id;
    std::vector<int16_t> channel_data;

    bool operator==(const NDTPPayloadBroadbandChannelData& other) const;
    bool operator!=(const NDTPPayloadBroadbandChannelData& other) const;
};

class NDTPPayloadBroadband {
public:
    NDTPPayloadBroadband(bool is_signed, uint32_t bit_width, uint32_t sample_rate,
                         const std::vector<NDTPPayloadBroadbandChannelData>& channels);

    bool is_signed;
    uint32_t bit_width;
    uint32_t sample_rate;
    std::vector<NDTPPayloadBroadbandChannelData> channels;

    std::vector<uint8_t> pack() const;
    static NDTPPayloadBroadband unpack(const std::vector<uint8_t>& data);

    bool operator==(const NDTPPayloadBroadband& other) const;
    bool operator!=(const NDTPPayloadBroadband& other) const;
};

class NDTPPayloadSpiketrain {
public:
    explicit NDTPPayloadSpiketrain(const std::vector<uint32_t>& spike_counts);

    std::vector<uint32_t> spike_counts;

    std::vector<uint8_t> pack() const;
    static NDTPPayloadSpiketrain unpack(const std::vector<uint8_t>& data);

    bool operator==(const NDTPPayloadSpiketrain& other) const;
    bool operator!=(const NDTPPayloadSpiketrain& other) const;
};


class NDTPHeader {
public:
    NDTPHeader(synapse::DataType data_type, uint64_t timestamp, uint32_t seq_number);

    synapse::DataType data_type;
    uint64_t timestamp;
    uint32_t seq_number;

    std::vector<uint8_t> pack() const;
    static NDTPHeader unpack(const std::vector<uint8_t>& data);

    bool operator==(const NDTPHeader& other) const;
    bool operator!=(const NDTPHeader& other) const;
};

class NDTPMessage {
public:
    NDTPMessage(const NDTPHeader& header, const std::shared_ptr<void>& payload);

    NDTPHeader header;
    std::shared_ptr<void> payload;


    uint16_t crc16(const std::vector<uint8_t>& data, uint16_t poly = 0x8005, uint16_t init = 0xFFFF) const;
    bool crc16_verify(const std::vector<uint8_t>& data, uint16_t crc) const;
    std::vector<uint8_t> pack() const;
    static NDTPMessage unpack(const std::vector<uint8_t>& data);

    bool operator==(const NDTPMessage& other) const;
    bool operator!=(const NDTPMessage& other) const;
};

} // namespace synapse
