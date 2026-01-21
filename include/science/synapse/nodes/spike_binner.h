#pragma once

#include <cstdint>
#include <memory>
#include "science/synapse/api/nodes/spike_binner.pb.h"
#include "science/synapse/node.h"

namespace synapse {

class SpikeBinner : public Node {
 public:
  explicit SpikeBinner(uint32_t bin_size_ms);

  [[nodiscard]] static auto from_proto(
    const synapse::NodeConfig& proto,
    std::shared_ptr<Node>* node
  ) -> science::Status;

 protected:
  auto p_to_proto(synapse::NodeConfig* proto) -> science::Status override;

 private:
  uint32_t bin_size_ms_;
};

}  // namespace synapse
