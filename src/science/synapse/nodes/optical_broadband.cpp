#include "science/synapse/nodes/optical_broadband.h"

namespace synapse {

OpticalBroadband::OpticalBroadband(
  uint32_t peripheral_id,
  std::optional<ChannelMask> pixel_mask,
  uint32_t bit_width,
  uint32_t frame_rate,
  uint32_t gain
) : Node(NodeType::kOpticalBroadband),
    peripheral_id_(peripheral_id),
    pixel_mask_(pixel_mask),
    bit_width_(bit_width),
    frame_rate_(frame_rate),
    gain_(gain) {}

auto OpticalBroadband::from_proto(const synapse::NodeConfig& proto, std::shared_ptr<Node>* node) -> science::Status {
  if (!proto.has_optical_broadband()) {
    return { science::StatusCode::kInvalidArgument, "missing optical_broadband config" };
  }

  const auto& config = proto.optical_broadband();
  std::optional<ChannelMask> pixel_mask;
  if (config.pixel_mask_size() > 0) {
    std::vector<uint32_t> indices(config.pixel_mask().begin(), config.pixel_mask().end());
    pixel_mask = ChannelMask(indices);
  }

  *node = std::make_shared<OpticalBroadband>(
    config.peripheral_id(),
    pixel_mask,
    config.bit_width(),
    config.frame_rate(),
    config.gain()
  );

  return {};
}

auto OpticalBroadband::p_to_proto(synapse::NodeConfig* proto) -> void {
  synapse::OpticalBroadbandConfig* config = proto->mutable_optical_broadband();

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
