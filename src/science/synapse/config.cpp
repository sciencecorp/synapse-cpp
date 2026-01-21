#include "science/synapse/config.h"
#include "science/synapse/nodes/broadband_source.h"
#include "science/synapse/nodes/disk_writer.h"
#include "science/synapse/nodes/electrical_stimulation.h"
#include "science/synapse/nodes/optical_stimulation.h"
#include "science/synapse/nodes/spectral_filter.h"
#include "science/synapse/nodes/spike_binner.h"
#include "science/synapse/nodes/spike_detector.h"
#include "science/synapse/nodes/spike_source.h"


namespace synapse {

auto create_node(const synapse::NodeConfig& config, std::shared_ptr<Node>* node_ptr) -> science::Status {
  if (node_ptr == nullptr) {
    return { science::StatusCode::kInvalidArgument, "node ptr must not be null" };
  }

  if (config.type() == synapse::NodeType::kNodeTypeUnknown) {
    return { science::StatusCode::kInvalidArgument, "unknown node type" };
  }

  switch (config.type()) {
    case synapse::NodeType::kBroadbandSource:
      return BroadbandSource::from_proto(config, node_ptr);

    case synapse::NodeType::kDiskWriter:
      return DiskWriter::from_proto(config, node_ptr);

    case synapse::NodeType::kElectricalStimulation:
      return ElectricalStimulation::from_proto(config, node_ptr);

    case synapse::NodeType::kOpticalStimulation:
      return OpticalStimulation::from_proto(config, node_ptr);

    case synapse::NodeType::kSpectralFilter:
      return SpectralFilter::from_proto(config, node_ptr);

    case synapse::NodeType::kSpikeBinner:
      return SpikeBinner::from_proto(config, node_ptr);

    case synapse::NodeType::kSpikeDetector:
      return SpikeDetector::from_proto(config, node_ptr);

    case synapse::NodeType::kSpikeSource:
      return SpikeSource::from_proto(config, node_ptr);

    default:
      return { science::StatusCode::kInvalidArgument, "unknown node type \"" + std::to_string(config.type()) + "\"" };
  }
}

auto Config::add(std::vector<std::shared_ptr<Node>> nodes) -> science::Status {
  science::Status s;
  for (auto& node : nodes) {
    if (node.get() == nullptr) {
      return { science::StatusCode::kInvalidArgument, "node must not be null" };
    }
    s = add_node(node);
    if (!s.ok()) {
      return s;
    }
  }
  return s;
}

auto Config::add_node(std::shared_ptr<Node> node, uint32_t id) -> science::Status {
  if (node.get() == nullptr) {
    return { science::StatusCode::kInvalidArgument, "node must not be null" };
  }

  if (node->id()) {
    return { science::StatusCode::kInvalidArgument, "node already has an id" };
  }

  if (id == 0) {
    id = gen_node_id();
  }

  for (auto& n : nodes_) {
    if (n->id() == id) {
      return { science::StatusCode::kInvalidArgument, "id already in use" };
    }
  }

  node->id_ = id;
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

auto Config::gen_node_id() -> uint32_t {
  return nodes_.size() + 1;
}

auto Config::set_device(const IDevice* device) -> science::Status {
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
    auto s = node->to_proto(config.add_nodes());
  }

  for (auto& [src, dst] : connections_) {
    auto connection = config.add_connections();
    connection->set_src_node_id(src);
    connection->set_dst_node_id(dst);
  }

  return config;
}

auto Config::from_proto(const synapse::DeviceConfiguration& proto, Config* config) -> science::Status {
  for (const auto& node_config : proto.nodes()) {
    std::shared_ptr<Node> node_ptr;
    auto s = create_node(node_config, &node_ptr);
    if (!s.ok()) {
      return s;
    }

    if (!node_ptr->id()) {
      node_ptr->id_ = node_config.id();
    }
    config->nodes_.push_back(node_ptr);
  }

  for (const auto& connection : proto.connections()) {
    auto src = std::find_if(config->nodes_.begin(), config->nodes_.end(), [&](auto& n) {
      return n->id() == connection.src_node_id();
    });
    auto dst = std::find_if(config->nodes_.begin(), config->nodes_.end(), [&](auto& n) {
      return n->id() == connection.dst_node_id();
    });

    if (src == config->nodes_.end()) {
      return { science::StatusCode::kInvalidArgument, "src node not found" };
    }

    if (dst == config->nodes_.end()) {
      return { science::StatusCode::kInvalidArgument, "dst node not found" };
    }

    config->connections_.push_back({ (*src)->id(), (*dst)->id() });
  }

  return {};
}

}  // namespace synapse
