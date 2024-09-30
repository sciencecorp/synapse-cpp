#pragma once

#include <memory>
#include <vector>
#include "science/synapse/api/channel.pb.h"
#include "science/synapse/api/nodes/electrical_stimulation.pb.h"
#include "science/synapse/channel.h"
#include "science/synapse/node.h"

namespace synapse {

class ElectricalStimulation : public Node {
 public:
  explicit ElectricalStimulation(
    uint32_t peripheral_id,
    const std::vector<Ch>& channels,
    uint32_t bit_width,
    uint32_t sample_rate,
    uint32_t lsb
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
  uint32_t lsb_;
};

}  // namespace synapse
