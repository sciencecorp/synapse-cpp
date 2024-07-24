#include "science/synapse/config.h"
#include "science/synapse/nodes/electrical_broadband.h"
#include "science/synapse/nodes/optical_stimulation.h"
#include "science/synapse/nodes/stream_in.h"
#include "science/synapse/nodes/stream_out.h"


namespace synapse {

auto Config::add(std::vector<std::shared_ptr<Node>> nodes) -> science::Status {
  science::Status s;
  for (auto& node : nodes) {
    s = add_node(node);
    if (!s.ok()) {
      return s;
    }
  }
  return s;
}

auto Config::add_node(std::shared_ptr<Node> node) -> science::Status {
  if (node->id()) {
    return { science::StatusCode::kInvalidArgument, "node already has an id" };
  }

  node->id_ = gen_node_id();
  nodes_.push_back(node);
  return {};
}

auto Config::connect(std::weak_ptr<Node> src, std::weak_ptr<Node> dst) -> science::Status {
  if (src.expired() || dst.expired()) {
    return { science::StatusCode::kInvalidArgument, "src or dst node is expired" };
  }
  auto src_node = src.lock();
  auto dst_node = dst.lock();
  if (!src_node->id() || !dst_node->id()) {
    return { science::StatusCode::kInvalidArgument, "src or dst node has no id" };
  }

  bool src_exists = false;
  bool dst_exists = false;
  for (auto& node : nodes_) {
    if (node->id() == src_node->id()) {
      src_exists = true;
    }
    if (node->id() == dst_node->id()) {
      dst_exists = true;
    }
  }

  if (!src_exists) {
    return { science::StatusCode::kInvalidArgument, "src node not found" };
  }

  if (!dst_exists) {
    return { science::StatusCode::kInvalidArgument, "dst node not found" };
  }

  auto exists = std::find_if(connections_.begin(), connections_.end(), [&](auto& c) {
    return c.first == src_node->id() && c.second == dst_node->id();
  }) != connections_.end();
  if (exists) {
    return { science::StatusCode::kInvalidArgument, "connection already exists" };
  }

  connections_.push_back({ src_node->id(), dst_node->id() });
  return {};
}

auto Config::gen_node_id() -> uint64_t {
  return nodes_.size() + 1;
}

auto Config::set_device(const Device* device) -> science::Status {
  if (device == nullptr) {
    return { science::StatusCode::kInvalidArgument, "device must not be null" };
  }
  science::Status s;
  for (auto& node : nodes_) {
    s = node->set_device(device);
    if (!s.ok()) {
      return s;
    }
  }
  return s;
}

auto Config::to_proto() -> synapse::DeviceConfiguration {
  synapse::DeviceConfiguration config;

  for (auto& node : nodes_) {
    node->to_proto(config.add_nodes());
  }

  for (auto& [src, dst] : connections_) {
    auto connection = config.add_connections();
    connection->set_src_node_id(src);
    connection->set_dst_node_id(dst);
  }

  return config;
}

}  // namespace synapse
