#include "science/synapse/nodes/stream_out.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "science/libndtp/ndtp.h"
#include "science/libndtp/types.h"
#include "science/synapse/device.h"

namespace synapse {

using science::libndtp::BinnedSpiketrainData;
using science::libndtp::ElectricalBroadbandData;
using science::libndtp::NDTPMessage;
using science::libndtp::SynapseData;

auto unpack(
  const std::vector<uint8_t>& bytes,
  SynapseData* data
) -> science::Status {
  NDTPMessage msg;
  try {
    msg = NDTPMessage::unpack(bytes);
  } catch (const std::exception& e) {
    std::cout << "Stream Out | error unpacking NDTP message: " << e.what() << std::endl;
    return { science::StatusCode::kInternal, "error unpacking NDTP message: " + std::string(e.what()) };
  }

  switch (msg.header.data_type) {
    case DataType::kBroadband:
      *data = ElectricalBroadbandData::unpack(msg);
      break;
    case DataType::kSpiketrain:
      *data = BinnedSpiketrainData::unpack(msg);
      break;
    default:
      return { science::StatusCode::kInternal, "unknown data type: " + std::to_string(msg.header.data_type) };
  }

  return {};
}

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
  auto s = UdpNode::init();
  if (!s.ok()) {
    return s;
  }

  int rc = 0;
  int on = 1;
  
  int flags = fcntl(sock(), F_GETFL, 0);
  if (flags < 0) {
    return {
      science::StatusCode::kInternal,
      "error getting socket flags (code: " + std::to_string(flags) + ")"
    };
  }
  rc = fcntl(sock(), F_SETFL, flags | O_NONBLOCK);
  if (rc < 0) {
    return {
      science::StatusCode::kInternal,
      "error setting non-blocking mode (code: " + std::to_string(rc) + ")"
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

auto StreamOut::read(SynapseData* data) -> science::Status {
  if (!sock() || !addr()) {
    auto s = init();
    if (!s.ok()) {
      return { s.code(), "error initializing socket: " + s.message() };
    }
  }

  auto saddr = addr().value();
  socklen_t saddr_len = sizeof(saddr);

  fd_set readfds;
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 1000;

  FD_ZERO(&readfds);
  FD_SET(sock(), &readfds);

  int ready = select(sock() + 1, &readfds, nullptr, nullptr, &tv);
  if (ready < 0) {
    return { science::StatusCode::kInternal, "error in select (code: " + std::to_string(errno) + ")" };
  }
  if (ready == 0) {
    return { science::StatusCode::kUnavailable, "no data available" };
  }

  std::vector<uint8_t> buf;
  buf.resize(8192);
  auto rc = recvfrom(sock(), buf.data(), buf.size(), 0, reinterpret_cast<sockaddr*>(&saddr), &saddr_len);
  if (rc < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return { science::StatusCode::kUnavailable, "no data available" };
    }
    buf.resize(0);
    return { science::StatusCode::kInternal, "error reading from socket (code: " + std::to_string(rc) + ")" };
  }

  buf.resize(rc);
  return unpack(buf, data);
}

auto StreamOut::p_to_proto(synapse::NodeConfig* proto) -> void {
  synapse::StreamOutConfig* config = proto->mutable_stream_out();

  config->set_label(label_);
  config->set_multicast_group(multicast_group_);
}

}  // namespace synapse
