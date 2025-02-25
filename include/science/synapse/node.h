#pragma once

#include <memory>
#include <string>
#include <vector>
#include <utility>

#include "science/scipp/status.h"
#include "science/synapse/api/node.pb.h"

namespace synapse {

class IDevice;

/**
 * Base Node class.
 * 
 * Represents a node in a Synapse device's signal chain. 
 */
class Node {
 public:
  explicit Node(const synapse::NodeType& node_type);
  virtual ~Node() = default;

  [[nodiscard]] auto id() const -> uint32_t;
  [[nodiscard]] auto set_device(const IDevice* device) -> science::Status;
  [[nodiscard]] auto to_proto(synapse::NodeConfig* proto) -> science::Status;

 protected:
  uint32_t id_;
  synapse::NodeType type_;
  const IDevice* device_;

  virtual auto p_to_proto(synapse::NodeConfig* proto) -> science::Status = 0;

  friend class Config;
};

}  // namespace synapse
