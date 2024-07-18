#include "science/synapse/nodes/optical_stimulation.h"

namespace synapse {

OpticalStimulation::OpticalStimulation(
  uint32_t peripheral_id,
  uint32_t sample_rate,
  uint32_t bit_width,
  uint32_t gain,
  std::optional<ChannelMask> pixel_mask
) : Node(NodeType::kStreamOut),
    peripheral_id_(peripheral_id),
    sample_rate_(sample_rate),
    bit_width_(bit_width),
    gain_(gain),
    pixel_mask_(pixel_mask) {}


auto OpticalStimulation::p_to_proto(synapse::NodeConfig* proto) -> void {
  synapse::OpticalStimConfig* config = proto->mutable_optical_stim();

  if (pixel_mask_.has_value()) {
    const auto& channels = pixel_mask_->channels();
    for (size_t c = 0; c < channels.size(); ++c) {
      if (!channels[c]) {
        continue;
      }
      config->add_pixel_mask(c);
    }
  }

  config->set_peripheral_id(peripheral_id_);
  config->set_sample_rate(sample_rate_);
  config->set_bit_width(bit_width_);
  config->set_gain(gain_);
}

}  // namespace synapse
