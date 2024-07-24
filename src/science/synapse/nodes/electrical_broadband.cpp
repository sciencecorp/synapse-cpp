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

auto ElectricalBroadband::from_proto(const synapse::NodeConfig& proto, std::shared_ptr<Node>* node) -> science::Status {
  if (!proto.has_electrical_broadband()) {
    return { science::StatusCode::kInvalidArgument, "missing electrical_broadband config" };
  }

  const auto& config = proto.electrical_broadband();
  std::optional<ChannelMask> ch_mask;
  if (config.ch_mask_size() > 0) {
    std::vector<uint32_t> indices(config.ch_mask().begin(), config.ch_mask().end());
    ch_mask = ChannelMask(indices);
  }

  *node = std::make_shared<ElectricalBroadband>(
    config.peripheral_id(),
    config.sample_rate(),
    config.bit_width(),
    config.gain(),
    ch_mask
  );

  return {};
}

auto ElectricalBroadband::p_to_proto(synapse::NodeConfig* proto) -> void {
  synapse::ElectricalBroadbandConfig* config = proto->mutable_electrical_broadband();

  if (channel_mask_.has_value()) {
    const auto& channels = channel_mask_->channels();
    for (const auto& c : channels) {
      config->add_ch_mask(c);
    }
  }

  config->set_peripheral_id(peripheral_id_);
  config->set_sample_rate(sample_rate_);
  config->set_bit_width(bit_width_);
  config->set_gain(gain_);
}

}  // namespace synapse
