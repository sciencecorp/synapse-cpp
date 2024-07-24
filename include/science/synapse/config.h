#pragma once

#include <memory>
#include <string>
#include <vector>
#include <utility>

#include "science/scipp/status.h"
#include "science/synapse/api/synapse.pb.h"
#include "science/synapse/node.h"

namespace synapse {

class Device;

/**
 * Configuration for a Synapse device.
 *
 * Contains configuration for any nodes on the Device, as well as their connections.
 * A Device must be configured before its signal chain can be run.
 */
class Config {
 public:
  Config() = default;

  /**
   * Add multiple nodes to the configuration.
   * The nodes will be assigned IDs.
   *
   * @param nodes Node configurations to add
   * @return science::Status
   */
  [[nodiscard]] auto add(std::vector<std::shared_ptr<Node>> nodes) -> science::Status;

  /**
   * Add a node to the configuration.
   * The node will be assigned an ID.
   *
   * @param node A node configuration to add
   * @return science::Status
   */
  [[nodiscard]] auto add_node(std::shared_ptr<Node> node) -> science::Status;

  /**
   * Connect the output of one node to the input of another.
   * Both nodes must be added to the Config before they can be connected.
   *
   * @param src The source node.
   * @param dst The destination node.
   * @return science::Status
   */
  [[nodiscard]] auto connect(std::weak_ptr<Node> src, std::weak_ptr<Node> dst) -> science::Status;

  /**
   * Get the connections in the configuration.
   *
   * @return const std::vector<std::pair<uint64_t, uint64_t>>&
   */
  [[nodiscard]] auto connections() const -> const std::vector<std::pair<uint64_t, uint64_t>>&;

  /**
   * Get the nodes in the configuration.
   *
   * @return const std::vector<std::shared_ptr<Node>>&
   */
  [[nodiscard]] auto nodes() const -> const std::vector<std::shared_ptr<Node>>&;

  /**
   * Convert the configuration to its corresponding DeviceConfiguration proto message.
   *
   * @return synapse::DeviceConfiguration
   */
  auto to_proto() -> synapse::DeviceConfiguration;

  /**
   * Convert the given DeviceConfiguration proto to its corresponding Config instance, with nodes and connections.
   *
   * @param proto The DeviceConfiguration proto message.
   * @param config The Config instance to populate.
   * @return science::Status
   */
  [[nodiscard]] static auto from_proto(const synapse::DeviceConfiguration& proto, Config* config) -> science::Status;

 protected:
  friend class Device;
  [[nodiscard]] auto set_device(const Device* device) -> science::Status;

 private:
  std::vector<std::shared_ptr<Node>> nodes_;
  std::vector<std::pair<uint64_t, uint64_t>> connections_;

  [[nodiscard]] auto gen_node_id() -> uint64_t;
};

}  // namespace synapse
