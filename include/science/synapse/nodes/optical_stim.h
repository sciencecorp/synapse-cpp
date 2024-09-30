#pragma once

#include <memory>
#include "science/synapse/api/nodes/optical_stim.pb.h"
#include "science/synapse/channel_mask.h"
#include "science/synapse/node.h"

namespace synapse {

class OpticalStim : public Node {
 public:
  explicit OpticalStim(
    uint32_t peripheral_id,
    std::optional<ChannelMask> pixel_mask,
    uint32_t bit_width,
    uint32_t frame_rate,
    uint32_t gain
  );

  [[nodiscard]] static auto from_proto(
    const synapse::NodeConfig& proto,
    std::shared_ptr<Node>* node
  ) -> science::Status;

 protected:
  auto p_to_proto(synapse::NodeConfig* proto) -> void override;

 private:
  uint32_t peripheral_id_;
  std::optional<ChannelMask> pixel_mask_;
  uint32_t bit_width_;
  uint32_t frame_rate_;
  uint32_t gain_;
};

}  // namespace synapse
