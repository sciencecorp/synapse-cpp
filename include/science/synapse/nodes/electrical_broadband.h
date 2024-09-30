#pragma once

#include <memory>
#include <vector>
#include "science/synapse/api/nodes/electrical_broadband.pb.h"
#include "science/synapse/channel.h"
#include "science/synapse/node.h"

namespace synapse {

class ElectricalBroadband : public Node {
 public:
  explicit ElectricalBroadband(
    uint32_t peripheral_id,
    const std::vector<Ch>& channels,
    uint32_t bit_width,
    uint32_t sample_rate,
    float gain,
    float low_cutoff_hz,
    float high_cutoff_hz
  );

  [[nodiscard]] static auto from_proto(
    const synapse::NodeConfig& proto,
    std::shared_ptr<Node>* node
  ) -> science::Status;


 protected:
  auto p_to_proto(synapse::NodeConfig* proto) -> void override;

 private:
  uint32_t peripheral_id_;
  std::vector<Ch> channels_;
  uint32_t bit_width_;
  uint32_t sample_rate_;
  uint32_t gain_;
  float low_cutoff_hz_;
  float high_cutoff_hz_;
};

}  // namespace synapse
