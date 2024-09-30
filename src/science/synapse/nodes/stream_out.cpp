#include "science/synapse/nodes/stream_out.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "science/synapse/device.h"

namespace synapse {

StreamOut::StreamOut(const std::string& label, const std::string& multicast_group) : UdpNode(NodeType::kStreamOut),
    label_(label),
    multicast_group_(multicast_group) {}

auto StreamOut::from_proto(const synapse::NodeConfig& proto, std::shared_ptr<Node>* node) -> science::Status {
  if (!proto.has_stream_out()) {
    return { science::StatusCode::kInvalidArgument, "missing stream_out config" };
  }

  const auto& config = proto.stream_out();
  const auto& label = config.label();

  if (config.multicast_group().empty()) {
    return { science::StatusCode::kInvalidArgument, "multicast_group is required but not set" };
  }
  const auto& multicast_group = config.multicast_group();

  *node = std::make_shared<StreamOut>(label, multicast_group);
  return {};
}

auto StreamOut::get_host(std::string* host) -> science::Status {
  if (multicast_group_.empty()) {
    return { science::StatusCode::kInvalidArgument, "multicast_group required but not set" };
  }
  *host = multicast_group_;
  return {};
}

auto StreamOut::init() -> science::Status {
  auto s  = UdpNode::init();
  if (!s.ok()) {
    return s;
  }

  int rc = 0;
  int on = 1;
  rc = setsockopt(sock(), SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
  if (rc < 0) {
    return {
      science::StatusCode::kInternal,
      "error configuring socket options (code: " + std::to_string(rc) + ")"
    };
  }

  rc = setsockopt(sock(), SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
  if (rc < 0) {
    return {
      science::StatusCode::kInternal,
      "error configuring socket options (code: " + std::to_string(rc) + ")"
    };
  }

  auto saddr = addr().value();

  std::cout << "binding to " << inet_ntoa(saddr.sin_addr) << ":" << ntohs(saddr.sin_port) << std::endl;
  rc = bind(sock(), reinterpret_cast<sockaddr*>(&saddr), sizeof(saddr));
  if (rc < 0) {
    return { science::StatusCode::kInternal, "error binding socket (code: " + std::to_string(rc) + ")" };
  }

  std::cout << "joining multicast group " << multicast_group_ << std::endl;
  ip_mreq mreq;
  mreq.imr_multiaddr.s_addr = inet_addr(multicast_group_.c_str());
  mreq.imr_interface.s_addr = htonl(INADDR_ANY);

  rc = setsockopt(sock(), IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
  if (rc < 0) {
    return { science::StatusCode::kInternal, "error joining multicast group (code: " + std::to_string(rc) + ")" };
  }

  return {};
}

auto StreamOut::read(std::vector<std::byte>* out) -> science::Status {
  if (!sock() || !addr()) {
    auto s = init();
    if (!s.ok()) {
      return { s.code(), "error initializing socket: " + s.message() };
    }
  }

  auto saddr = addr().value();
  socklen_t saddr_len = sizeof(saddr);

  out->clear();
  out->resize(1024);
  auto rc = recvfrom(sock(), out->data(), out->size(), 0, reinterpret_cast<sockaddr*>(&saddr), &saddr_len);
  if (rc < 0) {
    out->resize(0);
    return { science::StatusCode::kInternal, "error reading from socket (code: " + std::to_string(rc) + ")" };
  }

  out->resize(rc);
  return {};
}

auto StreamOut::p_to_proto(synapse::NodeConfig* proto) -> void {
  synapse::StreamOutConfig* config = proto->mutable_stream_out();

  config->set_label(label_);
  config->set_multicast_group(multicast_group_);
}

}  // namespace synapse
