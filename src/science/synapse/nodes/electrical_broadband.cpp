#include "science/synapse/nodes/electrical_broadband.h"

namespace synapse {

ElectricalBroadband::ElectricalBroadband(
  uint32_t peripheral_id,
  uint32_t sample_rate,
  uint32_t bit_width,
  uint32_t gain,
  std::optional<ChannelMask> channel_mask
) : Node(NodeType::kElectricalBroadband),
    peripheral_id_(peripheral_id),
    sample_rate_(sample_rate),
    bit_width_(bit_width),
    gain_(gain),
    channel_mask_(channel_mask) {}


auto ElectricalBroadband::p_to_proto(synapse::NodeConfig* proto) -> void {
  synapse::ElectricalBroadbandConfig* config = proto->mutable_electrical_broadband();

  if (channel_mask_.has_value()) {
    const auto& channels = channel_mask_->channels();
    for (size_t c = 0; c < channels.size(); ++c) {
      if (!channels[c]) {
        continue;
      }
      config->add_ch_mask(c);
    }
  }

  config->set_peripheral_id(peripheral_id_);
  config->set_sample_rate(sample_rate_);
  config->set_bit_width(bit_width_);
  config->set_gain(gain_);
}

}  // namespace synapse
