#pragma once

#include <memory>
#include <string>
#include <vector>
#include <utility>

#include "science/scipp/status.h"
#include "science/synapse/api/node.pb.h"

namespace synapse {

class Device;

/**
 * Base Node class.
 * 
 * Represents a node in a Synapse device's signal chain. 
 */
class Node {
 public:
  explicit Node(const synapse::NodeType& node_type);
  virtual ~Node() = default;

  [[nodiscard]] auto id() const -> uint64_t;
  [[nodiscard]] auto set_device(const Device* device) -> science::Status;
  auto to_proto(synapse::NodeConfig* proto) -> void;

 protected:
  uint64_t id_;
  synapse::NodeType type_;
  const Device* device_;

  virtual auto p_to_proto(synapse::NodeConfig* proto) -> void = 0;

  friend class Config;
};

}  // namespace synapse
