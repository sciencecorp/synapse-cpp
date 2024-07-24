#pragma once

#include <memory>
#include "science/synapse/api/nodes/electrical_stim.pb.h"
#include "science/synapse/channel_mask.h"
#include "science/synapse/node.h"

namespace synapse {

class ElectricalStim : public Node {
 public:
  explicit ElectricalStim(
    uint32_t peripheral_id,
    uint32_t sample_rate,
    uint32_t bit_width,
    uint32_t lsb,
    std::optional<ChannelMask> channel_mask
  );

  [[nodiscard]] static auto from_proto(
    const synapse::NodeConfig& proto,
    std::shared_ptr<Node>* node
  ) -> science::Status;


 protected:
  auto p_to_proto(synapse::NodeConfig* proto) -> void override;

 private:
  uint32_t peripheral_id_;
  uint32_t sample_rate_;
  uint32_t bit_width_;
  uint32_t lsb_;
  std::optional<ChannelMask> channel_mask_;
};

}  // namespace synapse
