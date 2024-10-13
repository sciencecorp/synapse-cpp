#pragma once

#include <vector>
#include <cstdint>
#include <memory>
#include "science/synapse/api/datatype.pb.h"
#include "science/synapse/util/ndtp.h"

namespace synapse {

class ElectricalBroadbandData {
public:
    ElectricalBroadbandData(uint64_t t0, size_t bit_width,
                            const std::vector<std::pair<int, std::vector<float>>>& samples,
                            int sample_rate, bool is_signed = true);

    auto pack(int seq_number) const -> std::vector<std::vector<uint8_t>>;

    static ElectricalBroadbandData from_ndtp_message(const NDTPMessage& msg);
    static ElectricalBroadbandData unpack(const std::vector<uint8_t>& data);

    // std::vector<std::variant<uint64_t, std::vector<std::pair<int, std::vector<float>>>>> to_list() const;

private:
    synapse::DataType data_type;
    uint64_t t0;
    bool is_signed;
    uint32_t bit_width;
    std::vector<std::pair<int, std::vector<float>>> samples;
    uint32_t sample_rate;
};

class SpiketrainData {
// public:
//     SpiketrainData(uint64_t t0, const std::vector<uint32_t>& spike_counts);

//     std::vector<std::vector<uint8_t>> pack(uint32_t seq_number) const;

//     static SpiketrainData from_ndtp_message(const NDTPMessage& msg);
//     static SpiketrainData unpack(const std::vector<uint8_t>& data);

//     std::vector<std::variant<uint64_t, std::vector<uint32_t>>> to_list() const;

// private:
//     synapse::DataType data_type;
//     uint64_t t0;
//     std::vector<uint32_t> spike_counts;
};

using SynapseData = std::variant<SpiketrainData, ElectricalBroadbandData>;

} // namespace synapse
