#pragma once

#include <cstdint>
#include <memory>
#include <variant>
#include <vector>
#include "science/synapse/api/nodes/broadband_source.pb.h"
#include "science/synapse/signal_config.h"
#include "science/synapse/node.h"

namespace synapse {

class BroadbandSource : public Node {
 public:
  explicit BroadbandSource(
    uint32_t peripheral_id,
    uint32_t bit_width,
    uint32_t sample_rate_hz,
    float gain,
    const Signal& signal
  );

  [[nodiscard]] static auto from_proto(
    const synapse::NodeConfig& proto,
    std::shared_ptr<Node>* node
  ) -> science::Status;


 protected:
  auto p_to_proto(synapse::NodeConfig* proto) -> science::Status override;

 private:
  uint32_t peripheral_id_;
  uint32_t bit_width_;
  uint32_t sample_rate_hz_;
  uint32_t gain_;
  Signal signal_;
};

}  // namespace synapse
