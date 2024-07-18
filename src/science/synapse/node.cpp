#include "science/synapse/node.h"

namespace synapse {

Node::Node(const NodeType& type) : type_(type) {}

auto Node::id() const -> uint64_t {
  return id_;
}

auto Node::set_device(const Device* device) -> void {
  device_ = device;
}

auto Node::to_proto(synapse::NodeConfig* proto) -> void {
  p_to_proto(proto);
  proto->set_id(id_);
  proto->set_type(type_);
}

}  // namespace synapse
