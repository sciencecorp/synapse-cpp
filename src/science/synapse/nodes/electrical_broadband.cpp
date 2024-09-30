#include "science/synapse/nodes/electrical_broadband.h"

namespace synapse {

ElectricalBroadband::ElectricalBroadband(
  uint32_t peripheral_id,
  const std::vector<Ch>& channels,
  uint32_t bit_width,
  uint32_t sample_rate,
  float gain,
  float low_cutoff_hz,
  float high_cutoff_hz
) : Node(NodeType::kElectricalBroadband),
    peripheral_id_(peripheral_id),
    channels_(channels),
    bit_width_(bit_width),
    sample_rate_(sample_rate),
    gain_(gain),
    low_cutoff_hz_(low_cutoff_hz),
    high_cutoff_hz_(high_cutoff_hz) {}

auto ElectricalBroadband::from_proto(const synapse::NodeConfig& proto, std::shared_ptr<Node>* node) -> science::Status {
  if (!proto.has_electrical_broadband()) {
    return { science::StatusCode::kInvalidArgument, "missing electrical_broadband config" };
  }

  const auto& config = proto.electrical_broadband();
  std::vector<Ch> channels;
  for (const auto& channel : config.channels()) {
    channels.push_back({
      channel.id(),
      channel.electrode_id(),
      channel.reference_id()
    });
  }

  *node = std::make_shared<ElectricalBroadband>(
    config.peripheral_id(),
    channels,
    config.bit_width(),
    config.sample_rate(),
    config.gain(),
    config.low_cutoff_hz(),
    config.high_cutoff_hz()
  );

  return {};
}

auto ElectricalBroadband::p_to_proto(synapse::NodeConfig* proto) -> void {
  synapse::ElectricalBroadbandConfig* config = proto->mutable_electrical_broadband();

  for (const auto& channel : channels_) {
    channel.to_proto(config->add_channels());
  }

  config->set_peripheral_id(peripheral_id_);
  config->set_bit_width(bit_width_);
  config->set_sample_rate(sample_rate_);
  config->set_gain(gain_);
  config->set_low_cutoff_hz(low_cutoff_hz_);
  config->set_high_cutoff_hz(high_cutoff_hz_);
}

}  // namespace synapse
