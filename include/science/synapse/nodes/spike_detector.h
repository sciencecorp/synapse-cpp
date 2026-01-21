#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include "science/synapse/api/nodes/spike_detector.pb.h"
#include "science/synapse/node.h"

namespace synapse {

class SpikeDetector : public Node {
 public:
  // Create a thresholder-based spike detector
  static auto create_thresholder(uint32_t threshold_uv, uint32_t samples_per_spike)
    -> std::shared_ptr<SpikeDetector>;

  // Create a template matcher-based spike detector
  static auto create_template_matcher(const std::vector<uint32_t>& template_uv, uint32_t samples_per_spike)
    -> std::shared_ptr<SpikeDetector>;

  [[nodiscard]] static auto from_proto(
    const synapse::NodeConfig& proto,
    std::shared_ptr<Node>* node
  ) -> science::Status;

 protected:
  auto p_to_proto(synapse::NodeConfig* proto) -> science::Status override;

 private:
  SpikeDetector();

  enum class Mode { Thresholder, TemplateMatcher };
  Mode mode_;
  uint32_t threshold_uv_;
  std::vector<uint32_t> template_uv_;
  uint32_t samples_per_spike_;
};

}  // namespace synapse
