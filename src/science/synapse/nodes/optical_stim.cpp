#include "science/synapse/nodes/optical_stim.h"

namespace synapse {

OpticalStim::OpticalStim(
  uint32_t peripheral_id,
  std::optional<ChannelMask> pixel_mask,
  uint32_t bit_width,
  uint32_t frame_rate,
  uint32_t gain
) : Node(NodeType::kOpticalStim),
    peripheral_id_(peripheral_id),
    pixel_mask_(pixel_mask),
    bit_width_(bit_width),
    frame_rate_(frame_rate),
    gain_(gain) {}

auto OpticalStim::from_proto(const synapse::NodeConfig& proto, std::shared_ptr<Node>* node) -> science::Status {
  if (!proto.has_optical_stim()) {
    return { science::StatusCode::kInvalidArgument, "missing optical_stim config" };
  }

  const auto& config = proto.optical_stim();
  std::optional<ChannelMask> pixel_mask;
  if (config.pixel_mask_size() > 0) {
    std::vector<uint32_t> indices(config.pixel_mask().begin(), config.pixel_mask().end());
    pixel_mask = ChannelMask(indices);
  }

  *node = std::make_shared<OpticalStim>(
    config.peripheral_id(),
    pixel_mask,
    config.bit_width(),
    config.frame_rate(),
    config.gain()
  );

  return {};
}

auto OpticalStim::p_to_proto(synapse::NodeConfig* proto) -> void {
  synapse::OpticalStimConfig* config = proto->mutable_optical_stim();

  if (pixel_mask_.has_value()) {
    const auto& channels = pixel_mask_->channels();
    for (const auto& c : channels) {
      config->add_pixel_mask(c);
    }
  }

  config->set_peripheral_id(peripheral_id_);
  config->set_bit_width(bit_width_);
  config->set_frame_rate(frame_rate_);
  config->set_gain(gain_);
}

}  // namespace synapse
