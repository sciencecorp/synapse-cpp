#include "science/synapse/util/ndtp_types.h"
#include <algorithm>

namespace synapse {

ElectricalBroadbandData::ElectricalBroadbandData(uint64_t t0, size_t bit_width,
                                                 const std::vector<std::pair<int, std::vector<float>>>& samples,
                                                 int sample_rate, bool is_signed)
    : data_type(DataType::kBroadband), t0(t0), is_signed(is_signed), bit_width(bit_width),
      samples(samples), sample_rate(sample_rate) {}

auto ElectricalBroadbandData::pack(int seq_number) const -> std::vector<std::vector<uint8_t>> {
    std::vector<std::vector<uint8_t>> packets;
    uint32_t seq_number_offset = 0;

    for (const auto& ch_samples : samples) {
        NDTPMessage message(
            NDTPHeader(DataType::kBroadband, t0, seq_number + seq_number_offset),
            NDTPPayloadBroadband(
                is_signed,
                bit_width,
                sample_rate,
                std::vector<NDTPPayloadBroadbandChannelData>{
                    NDTPPayloadBroadbandChannelData(ch_samples.first,
                        std::vector<float>(ch_samples.second.begin(), ch_samples.second.end()))
                }
            )
        );
        packets.push_back(message.pack());
        seq_number_offset++;
    }

    return packets;
}

ElectricalBroadbandData ElectricalBroadbandData::from_ndtp_message(const NDTPMessage& msg) {
    const auto& broadband_payload = std::get<NDTPPayloadBroadband>(msg.payload);

    std::vector<std::pair<int, std::vector<float>>> samples;
    for (const auto& ch : broadband_payload.channels) {
        samples.emplace_back(ch.channel_id, std::vector<float>(ch.channel_data.begin(), ch.channel_data.end()));
    }
    return ElectricalBroadbandData(msg.header.timestamp, broadband_payload.bit_width, samples, broadband_payload.sample_rate);
}

ElectricalBroadbandData ElectricalBroadbandData::unpack(const std::vector<uint8_t>& data) {
    NDTPMessage u = NDTPMessage::unpack(data);
    return from_ndtp_message(u);
}

// std::vector<std::variant<uint64_t, std::vector<std::pair<uint32_t, std::vector<uint16_t>>>> ElectricalBroadbandData::to_list() const {
//     std::vector<std::pair<uint32_t, std::vector<uint16_t>>> sample_list;
//     for (const auto& [channel_id, samples] : this->samples) {
//         sample_list.emplace_back(channel_id, samples);
//     }
//     return {t0, sample_list};
// }

// SpiketrainData::SpiketrainData(uint64_t t0, const std::vector<uint32_t>& spike_counts)
//     : data_type(DataType::kSpiketrain), t0(t0), spike_counts(spike_counts) {}

// std::vector<std::vector<uint8_t>> SpiketrainData::pack(uint32_t seq_number) const {
//     NDTPMessage message(
//         NDTPHeader(DataType::kSpiketrain, t0, seq_number),
//         std::make_shared<NDTPPayloadSpiketrain>(spike_counts)
//     );
//     return {message.pack()};
// }

// SpiketrainData SpiketrainData::from_ndtp_message(const NDTPMessage& msg) {
//     auto payload = std::static_pointer_cast<NDTPPayloadSpiketrain>(msg.payload);
//     return SpiketrainData(msg.header.timestamp, payload->spike_counts);
// }

// SpiketrainData SpiketrainData::unpack(const std::vector<uint8_t>& data) {
//     NDTPMessage u = NDTPMessage::unpack(data);
//     return from_ndtp_message(u);
// }

// std::vector<std::variant<uint64_t, std::vector<uint32_t>>> SpiketrainData::to_list() const {
//     return {t0, spike_counts};
// }

} // namespace synapse
