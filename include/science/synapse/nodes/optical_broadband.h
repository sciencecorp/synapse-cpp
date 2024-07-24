#pragma once

#include <memory>
#include "science/synapse/api/nodes/optical_broadband.pb.h"
#include "science/synapse/channel_mask.h"
#include "science/synapse/node.h"

namespace synapse {

class OpticalBroadband : public Node {
 public:
  explicit OpticalBroadband(
    uint32_t sample_rate,
    uint32_t bit_width,
    uint32_t gain,
    std::optional<ChannelMask> pixel_mask
  );

  [[nodiscard]] static auto from_proto(
    const synapse::NodeConfig& proto,
    std::shared_ptr<Node>* node
  ) -> science::Status;


 protected:
  auto p_to_proto(synapse::NodeConfig* proto) -> void override;

 private:
  uint32_t sample_rate_;
  uint32_t bit_width_;
  uint32_t gain_;
  std::optional<ChannelMask> pixel_mask_;
};

}  // namespace synapse
