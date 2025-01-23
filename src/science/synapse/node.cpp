#include "science/synapse/node.h"

namespace synapse {

Node::Node(const NodeType& type) : type_(type) {}

auto Node::id() const -> uint32_t {
  return id_;
}

auto Node::set_device(const Device* device) -> science::Status {
  if (device == nullptr) {
    return {
        science::StatusCode::kInvalidArgument,
        "device must not be null",
    };
  }
  device_ = device;
  return {};
}

auto Node::to_proto(synapse::NodeConfig* proto) -> void {
  p_to_proto(proto);
  proto->set_id(id_);
  proto->set_type(type_);
}

}  // namespace synapse
