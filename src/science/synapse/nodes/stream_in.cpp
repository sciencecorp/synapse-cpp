#include "science/synapse/nodes/stream_in.h"

namespace synapse {

StreamIn::StreamIn(std::optional<std::string> multicast_group)
  : Node(NodeType::kStreamIn),
    multicast_group_(multicast_group) {}

auto StreamIn::write(std::vector<std::byte> bytes) const -> bool {
  return {};
}

auto StreamIn::p_to_proto(synapse::NodeConfig* proto) -> void {
  synapse::StreamInConfig* config = proto->mutable_stream_in();

  if (multicast_group_.has_value()) {
    config->set_multicast_group(multicast_group_.value());
  }
}

}  // namespace synapse
