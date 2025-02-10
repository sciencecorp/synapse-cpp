#include "science/synapse/nodes/stream_in.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "science/synapse/device.h"

namespace synapse {

StreamIn::StreamIn(const synapse::DataType& data_type, const std::vector<uint32_t>& shape)
  : UdpNode(NodeType::kStreamIn), data_type_(data_type), shape_(shape)  {}

auto StreamIn::from_proto(const synapse::NodeConfig& proto, std::shared_ptr<Node>* node) -> science::Status {
  if (!proto.has_stream_in()) {
    return { science::StatusCode::kInvalidArgument, "missing stream_in config" };
  }

  const auto& config = proto.stream_in();
  auto data_type = config.data_type();
  auto shape = std::vector<uint32_t>(config.shape().begin(), config.shape().end());
  *node = std::make_shared<StreamIn>(data_type, shape);
  return {};
}

auto StreamIn::write(const std::vector<std::byte>& in) -> science::Status {
  if (!sock() || !addr()) {
    auto s = UdpNode::init();
    if (!s.ok()) {
      return { s.code(), "error initializing socket: " + s.message() };
    }
  }

  const auto saddr = addr().value();
  sendto(sock(), in.data(), in.size(), 0, reinterpret_cast<const sockaddr*>(&saddr), sizeof(saddr));

  return {};
}

auto StreamIn::p_to_proto(synapse::NodeConfig* proto) -> science::Status {
  if (proto == nullptr) {
    return { science::StatusCode::kInvalidArgument, "proto ptr must not be null" };
  }

  synapse::StreamInConfig* config = proto->mutable_stream_in();

  config->set_data_type(data_type_);
  for (const auto& dim : shape_) {
    config->add_shape(dim);
  }

  return {};
}

}  // namespace synapse
