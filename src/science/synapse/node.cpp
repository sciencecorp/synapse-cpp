#include "science/synapse/node.h"

namespace synapse {

Node::Node(const NodeType& type) : id_(0), type_(type) {}

auto Node::id() const -> uint32_t {
  return id_;
}

auto Node::set_device(const Device* device) -> science::Status {
  if (device == nullptr) {
    return { science::StatusCode::kInvalidArgument, "device ptr must not be null" };
  }
  device_ = device;
  return {};
}

auto Node::to_proto(synapse::NodeConfig* proto) -> science::Status {
  if (proto == nullptr) {
    return { science::StatusCode::kInvalidArgument, "proto ptr must not be null" };
  }
  proto->set_id(id_);
  proto->set_type(type_);
  auto s = p_to_proto(proto);
  return s;
}

}  // namespace synapse
