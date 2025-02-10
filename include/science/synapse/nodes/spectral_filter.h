#pragma once

#include <memory>
#include "science/synapse/api/nodes/spectral_filter.pb.h"
#include "science/synapse/node.h"

namespace synapse {

class SpectralFilter : public Node {
 public:
  explicit SpectralFilter(
    const synapse::SpectralFilterMethod& method,
    uint32_t low_cutoff_hz,
    uint32_t high_cutoff_hz
  );

  [[nodiscard]] static auto from_proto(
    const synapse::NodeConfig& proto,
    std::shared_ptr<Node>* node
  ) -> science::Status;


 protected:
  auto p_to_proto(synapse::NodeConfig* proto) -> science::Status override;

 private:
  synapse::SpectralFilterMethod method_;
  uint32_t low_cutoff_hz_;
  uint32_t high_cutoff_hz_;
};

}  // namespace synapse
