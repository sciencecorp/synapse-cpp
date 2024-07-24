#pragma once

#include <memory>
#include "science/synapse/api/nodes/spectral_filter.pb.h"
#include "science/synapse/node.h"

namespace synapse {

class SpectralFilter : public Node {
 public:
  explicit SpectralFilter(
    const synapse::SpectralFilterMethod& method,
    uint32_t low_cutoff_mhz,
    uint32_t high_cutoff_mhz
  );

  [[nodiscard]] static auto from_proto(
    const synapse::NodeConfig& proto,
    std::shared_ptr<Node>* node
  ) -> science::Status;


 protected:
  auto p_to_proto(synapse::NodeConfig* proto) -> void override;

 private:
  synapse::SpectralFilterMethod method_;
  uint32_t low_cutoff_mhz_;
  uint32_t high_cutoff_mhz_;
};

}  // namespace synapse
