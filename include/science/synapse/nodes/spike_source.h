#pragma once

#include <cstdint>
#include <memory>
#include <variant>
#include <vector>
#include "science/synapse/api/nodes/spike_source.pb.h"
#include "science/synapse/signal_config.h"
#include "science/synapse/node.h"

namespace synapse {

class SpikeSource : public Node {
 public:
  explicit SpikeSource(
    uint32_t peripheral_id,
    uint32_t sample_rate_hz,
    float spike_window_ms,
    float gain,
    float threshold_uV,
    const Electrodes& electrodes_config
  );

  [[nodiscard]] static auto from_proto(
    const synapse::NodeConfig& proto,
    std::shared_ptr<Node>* node
  ) -> science::Status;


 protected:
  auto p_to_proto(synapse::NodeConfig* proto) -> science::Status override;

 private:
  uint32_t peripheral_id_;
  uint32_t sample_rate_hz_;
  float spike_window_ms_;
  float gain_;
  float threshold_uV_;
  Electrodes electrodes_config_;
};

}  // namespace synapse
