#include "science/synapse/nodes/spectral_filter.h"

namespace synapse {

SpectralFilter::SpectralFilter(
  const synapse::SpectralFilterMethod& method,
  uint32_t low_cutoff_mhz,
  uint32_t high_cutoff_mhz
) : Node(NodeType::kSpectralFilter),
    method_(method),
    low_cutoff_mhz_(low_cutoff_mhz),
    high_cutoff_mhz_(high_cutoff_mhz) {}

auto SpectralFilter::from_proto(const synapse::NodeConfig& proto, std::shared_ptr<Node>* node) -> science::Status {
  if (!proto.has_spectral_filter()) {
    return { science::StatusCode::kInvalidArgument, "missing spectral_filter config" };
  }

  const auto& config = proto.spectral_filter();

  *node = std::make_shared<SpectralFilter>(
    config.method(),
    config.low_cutoff_mhz(),
    config.high_cutoff_mhz()
  );

  return {};
}

auto SpectralFilter::p_to_proto(synapse::NodeConfig* proto) -> void {
  synapse::SpectralFilterConfig* config = proto->mutable_spectral_filter();

  config->set_method(method_);
  config->set_low_cutoff_mhz(low_cutoff_mhz_);
  config->set_high_cutoff_mhz(high_cutoff_mhz_);
}

}  // namespace synapse
