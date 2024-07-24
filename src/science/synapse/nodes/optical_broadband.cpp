#include "science/synapse/nodes/optical_broadband.h"

namespace synapse {

OpticalBroadband::OpticalBroadband(
  uint32_t sample_rate,
  uint32_t bit_width,
  uint32_t gain,
  std::optional<ChannelMask> pixel_mask
) : Node(NodeType::kOpticalBroadband),
    sample_rate_(sample_rate),
    bit_width_(bit_width),
    gain_(gain),
    pixel_mask_(pixel_mask) {}

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
    config.sample_rate(),
    config.bit_width(),
    config.gain(),
    pixel_mask
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

  config->set_sample_rate(sample_rate_);
  config->set_bit_width(bit_width_);
  config->set_gain(gain_);
}

}  // namespace synapse
