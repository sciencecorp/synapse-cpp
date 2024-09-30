#include "science/synapse/nodes/electrical_stimulation.h"

namespace synapse {

ElectricalStimulation::ElectricalStimulation(
  uint32_t peripheral_id,
  const std::vector<Ch>& channels,
  uint32_t bit_width,
  uint32_t sample_rate,
  uint32_t lsb
) : Node(NodeType::kElectricalStimulation),
    peripheral_id_(peripheral_id),
    channels_(channels),
    bit_width_(bit_width),
    sample_rate_(sample_rate),
    lsb_(lsb) {}

auto ElectricalStimulation::from_proto(const synapse::NodeConfig& proto, std::shared_ptr<Node>* node) -> science::Status {
  if (!proto.has_electrical_stimulation()) {
    return { science::StatusCode::kInvalidArgument, "missing electrical_stimulation config" };
  }

  const auto& config = proto.electrical_stimulation();
  std::vector<Ch> channels;
  for (const auto& channel : config.channels()) {
    channels.push_back({
      channel.id(),
      channel.electrode_id(),
      channel.reference_id()
    });
  }

  *node = std::make_shared<ElectricalStimulation>(
    config.peripheral_id(),
    channels,
    config.bit_width(),
    config.sample_rate(),
    config.lsb()
  );

  return {};
}

auto ElectricalStimulation::p_to_proto(synapse::NodeConfig* proto) -> void {
  synapse::ElectricalStimulationConfig* config = proto->mutable_electrical_stimulation();

  for (const auto& channel : channels_) {
    channel.to_proto(config->add_channels());
  }


  config->set_peripheral_id(peripheral_id_);
  config->set_bit_width(bit_width_);
  config->set_sample_rate(sample_rate_);
  config->set_lsb(lsb_);
}

}  // namespace synapse
