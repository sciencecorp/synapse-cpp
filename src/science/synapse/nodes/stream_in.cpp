#include "science/synapse/nodes/stream_in.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "science/synapse/device.h"

namespace synapse {

StreamIn::StreamIn() : UdpNode(NodeType::kStreamIn) {}

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

auto StreamIn::p_to_proto(synapse::NodeConfig* proto) -> void {
  synapse::StreamInConfig* config = proto->mutable_stream_in();
}

}  // namespace synapse
