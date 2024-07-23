#include "science/synapse/nodes/stream_out.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "science/synapse/device.h"

namespace synapse {

StreamOut::StreamOut(
  const synapse::DataType& data_type,
  const std::vector<uint32_t>& shape,
  std::optional<std::string> multicast_group
) : UdpNode(NodeType::kStreamOut),
    data_type_(data_type),
    shape_(shape),
    multicast_group_(multicast_group) {}

auto StreamOut::get_host(std::string* host) -> science::Status {
  if (multicast_group_) {
    *host = multicast_group_.value();
    return {};
  }

  return UdpNode::get_host(host);
}

auto StreamOut::init() -> science::Status {
  std::cout << "Initializing socket" << std::endl;

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
  std::cout << "binding to: " << inet_ntoa(saddr.sin_addr) << ":" << ntohs(saddr.sin_port) << std::endl;

  rc = bind(sock(), reinterpret_cast<sockaddr*>(&saddr), sizeof(saddr));
  if (rc < 0) {
    return { science::StatusCode::kInternal, "error binding socket (code: " + std::to_string(rc) + ")" };
  }

  if (multicast_group_.has_value()) {
    ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(multicast_group_.value().c_str());
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    rc = setsockopt(sock(), IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
    if (rc < 0) {
      return { science::StatusCode::kInternal, "error joining multicast group (code: " + std::to_string(rc) + ")" };
    }
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

  config->set_data_type(data_type_);

  for (const auto& dim : shape_) {
    config->add_shape(dim);
  }

  if (multicast_group_.has_value()) {
    config->set_multicast_group(multicast_group_.value());
    config->set_use_multicast(true);
  }
}

}  // namespace synapse
