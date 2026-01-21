#include "science/synapse/nodes/spike_binner.h"

namespace synapse {

SpikeBinner::SpikeBinner(uint32_t bin_size_ms)
    : Node(NodeType::kSpikeBinner),
      bin_size_ms_(bin_size_ms) {}

auto SpikeBinner::from_proto(const synapse::NodeConfig& proto,
                             std::shared_ptr<Node>* node) -> science::Status {
  if (!proto.has_spike_binner()) {
    return {science::StatusCode::kInvalidArgument, "missing spike_binner config"};
  }

  const auto& config = proto.spike_binner();

  *node = std::make_shared<SpikeBinner>(config.bin_size_ms());

  return {};
}

auto SpikeBinner::p_to_proto(synapse::NodeConfig* proto) -> science::Status {
  if (proto == nullptr) {
    return {science::StatusCode::kInvalidArgument, "proto ptr must not be null"};
  }

  synapse::SpikeBinnerConfig* config = proto->mutable_spike_binner();
  config->set_bin_size_ms(bin_size_ms_);

  return {};
}

}  // namespace synapse
