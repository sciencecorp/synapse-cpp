#include "science/synapse/nodes/udp_node.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "science/synapse/device.h"

namespace synapse {


auto sockaddr(const std::string& host, uint16_t port) -> sockaddr_in {
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  inet_pton(AF_INET, host.c_str(), &addr.sin_addr);
  return addr;
}

UdpNode::UdpNode(const synapse::NodeType& node_type) : Node(node_type) {}

UdpNode::~UdpNode() {
  if (socket_ > 0) {
    close(socket_);
  }
}

auto UdpNode::addr() const -> std::optional<sockaddr_in> {
  return addr_;
}

auto UdpNode::get_host(std::string* host) -> science::Status {
  if (host == nullptr) {
    return { science::StatusCode::kInvalidArgument, "host ptr must not be null" };
  }

  if (device_ == nullptr) {
    return { science::StatusCode::kFailedPrecondition, "device not set, has Device been configured?" };
  }
  const auto& uri = device_->uri();
  auto pos = uri.find(':');
  if (pos == std::string::npos) {
    return { science::StatusCode::kInternal, "invalid uri on Device" };
  }

  *host = uri.substr(0, pos);
  return {};
}

auto UdpNode::get_port(uint16_t* port) -> science::Status {
  if (port == nullptr) {
    return { science::StatusCode::kInvalidArgument, "port ptr must not be null" };
  }

  if (device_ == nullptr) {
    return { science::StatusCode::kFailedPrecondition, "device not set, has Device been configured?" };
  }

  const auto& sockets = device_->sockets();
  const auto& self = std::find_if(sockets.begin(), sockets.end(), [this](const auto& s) {
    return s.node_id() == id();
  });

  if (self == sockets.end()) {
    return { science::StatusCode::kFailedPrecondition, "socket not found, has Device been configured?" };
  }

  auto addr = self->bind();

  auto pos = addr.find(':');

  if (pos == std::string::npos) {
    return { science::StatusCode::kInvalidArgument, "invalid bind address" };
  }

  try {
    *port = std::stoi(addr.substr(pos + 1));
  } catch (const std::invalid_argument& e) {
    return { science::StatusCode::kInvalidArgument, "invalid bind port" };
  } catch (const std::out_of_range& e) {
    return { science::StatusCode::kInternal, "bind port out of range" };
  }

  return {};
}

auto UdpNode::init() -> science::Status {
  std::string host;
  uint16_t port;

  auto s = get_host(&host);
  if (!s.ok()) {
    return { s.code(), "error initializing socket: " + s.message() };
  }

  s = get_port(&port);
  if (!s.ok()) {
    return { s.code(), "error initializing socket: " + s.message() };
  }

  addr_ = sockaddr(host, port);

  socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (socket_ < 0) {
    return { science::StatusCode::kInternal, "error creating socket (code: " + std::to_string(socket_) + ")" };
  }

  int reuse = 1;
  auto rc = setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
  if (rc < 0) {
    return { science::StatusCode::kInternal, "error configuring SO_REUSEADDR (code: " + std::to_string(rc) + ")" };
  }

  #ifdef SO_REUSEPORT
  rc = setsockopt(socket_, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));
  if (rc < 0) {
    return { science::StatusCode::kInternal, "error configuring SO_REUSEPORT (code: " + std::to_string(rc) + ")" };
  }
  #endif

  return {};
}

auto UdpNode::sock() const -> int {
  return socket_;
}


}  // namespace synapse
