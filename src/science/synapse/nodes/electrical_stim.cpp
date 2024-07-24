#include "science/synapse/nodes/electrical_stim.h"

namespace synapse {

ElectricalStim::ElectricalStim(
  uint32_t peripheral_id,
  uint32_t sample_rate,
  uint32_t bit_width,
  uint32_t lsb,
  std::optional<ChannelMask> channel_mask
) : Node(NodeType::kElectricalStim),
    peripheral_id_(peripheral_id),
    sample_rate_(sample_rate),
    bit_width_(bit_width),
    lsb_(lsb),
    channel_mask_(channel_mask) {}

auto ElectricalStim::from_proto(const synapse::NodeConfig& proto, std::shared_ptr<Node>* node) -> science::Status {
  if (!proto.has_electrical_stim()) {
    return { science::StatusCode::kInvalidArgument, "missing electrical_stim config" };
  }

  const auto& config = proto.electrical_stim();
  std::optional<ChannelMask> ch_mask;
  if (config.ch_mask_size() > 0) {
    std::vector<uint32_t> indices(config.ch_mask().begin(), config.ch_mask().end());
    ch_mask = ChannelMask(indices);
  }

  *node = std::make_shared<ElectricalStim>(
    config.peripheral_id(),
    config.sample_rate(),
    config.bit_width(),
    config.lsb(),
    ch_mask
  );

  return {};
}

auto ElectricalStim::p_to_proto(synapse::NodeConfig* proto) -> void {
  synapse::ElectricalStimConfig* config = proto->mutable_electrical_stim();

  if (channel_mask_.has_value()) {
    const auto& channels = channel_mask_->channels();
    for (const auto& c : channels) {
      config->add_ch_mask(c);
    }
  }

  config->set_peripheral_id(peripheral_id_);
  config->set_sample_rate(sample_rate_);
  config->set_bit_width(bit_width_);
  config->set_lsb(lsb_);
}

}  // namespace synapse
