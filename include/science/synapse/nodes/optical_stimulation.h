#pragma once

#include "science/synapse/api/nodes/optical_stim.pb.h"
#include "science/synapse/channel_mask.h"
#include "science/synapse/node.h"

namespace synapse {

class OpticalStimulation : public Node {
 public:
  explicit OpticalStimulation(
    uint32_t peripheral_id,
    uint32_t sample_rate,
    uint32_t bit_width,
    uint32_t gain,
    std::optional<ChannelMask> pixel_mask
  );

 protected:
  auto p_to_proto(synapse::NodeConfig* proto) -> void override;

 private:
  uint32_t peripheral_id_;
  uint32_t sample_rate_;
  uint32_t bit_width_;
  uint32_t gain_;
  std::optional<ChannelMask> pixel_mask_;
};

}  // namespace synapse
