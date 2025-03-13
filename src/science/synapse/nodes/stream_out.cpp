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

const std::string LOCALHOST = "127.0.0.1";

static auto get_client_ip() -> std::string {
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    return LOCALHOST;
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(80);
  addr.sin_addr.s_addr = inet_addr("8.8.8.8");

  if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    close(sock);
    return LOCALHOST;
  }

  struct sockaddr_in local_addr;
  socklen_t len = sizeof(local_addr);
  if (getsockname(sock, (struct sockaddr*)&local_addr, &len) < 0) {
    close(sock);
    return LOCALHOST;
  }

  close(sock);
  return inet_ntoa(local_addr.sin_addr);
}

static auto sockaddr(const std::string& host, uint16_t port) -> sockaddr_in {
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  inet_pton(AF_INET, host.c_str(), &addr.sin_addr);
  return addr;
}

auto unpack(
  const std::vector<uint8_t>& bytes,
  SynapseData* data,
  science::libndtp::NDTPHeader* header,
  size_t* bytes_read
) -> science::Status {
  NDTPMessage msg;
  try {
    msg = NDTPMessage::unpack(bytes);
  } catch (const std::exception& e) {
    return { science::StatusCode::kInternal, "error unpacking NDTP message: " + std::string(e.what()) };
  }

  if (header != nullptr) {
    *header = msg.header;
  }
  if (bytes_read != nullptr) {
    *bytes_read = bytes.size();
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

StreamOut::StreamOut(const std::string& destination_address,
                    uint16_t destination_port,
                    const std::string& label)
    : Node(NodeType::kStreamOut),
      destination_address_(destination_address.empty() ? get_client_ip() : destination_address),
      destination_port_(destination_port ? destination_port : DEFAULT_STREAM_OUT_PORT),
      label_(label) {}

StreamOut::~StreamOut() {
  if (socket_ > 0) {
    close(socket_);
  }
}

auto StreamOut::init() -> science::Status {
  socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (socket_ < 0) {
    return { science::StatusCode::kInternal, "error creating socket (code: " + std::to_string(socket_) + ")" };
  }

  // Allow reuse for easy restart
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

  // Set non-blocking mode
  int flags = fcntl(socket_, F_GETFL, 0);
  if (flags < 0) {
    return {
      science::StatusCode::kInternal,
      "error getting socket flags (code: " + std::to_string(flags) + ")"
    };
  }
  rc = fcntl(socket_, F_SETFL, flags | O_NONBLOCK);
  if (rc < 0) {
    return {
      science::StatusCode::kInternal,
      "error setting non-blocking mode (code: " + std::to_string(rc) + ")"
    };
  }

  // Try to set a large recv buffer
  int bufsize = SOCKET_BUFSIZE_BYTES;
  rc = setsockopt(socket_, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));
  if (rc < 0) {
    // continue
  }

  int actual_bufsize;
  socklen_t size = sizeof(actual_bufsize);
  rc = getsockopt(socket_, SOL_SOCKET, SO_RCVBUF, &actual_bufsize, &size);
  if (rc == 0 && actual_bufsize < SOCKET_BUFSIZE_BYTES) {
    // continue
  }

  addr_ = sockaddr(destination_address_, destination_port_);
  
  rc = bind(socket_, reinterpret_cast<struct sockaddr*>(&addr_.value()), sizeof(addr_.value()));
  if (rc < 0) {
    return { science::StatusCode::kInternal, 
             "error binding socket to " + destination_address_ + ":" + std::to_string(destination_port_) +
             " (code: " + std::to_string(rc) + ", errno: " + std::to_string(errno) + ")" };
  }

  return {};
}

auto StreamOut::read(SynapseData* data, science::libndtp::NDTPHeader* header, size_t* bytes_read) -> science::Status {
  if (!socket_ || !addr_) {
    auto s = init();
    if (!s.ok()) {
      return { s.code(), "error initializing socket: " + s.message() };
    }
  }

  auto saddr = addr_.value();
  socklen_t saddr_len = sizeof(saddr);

  fd_set readfds;
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 1000;

  FD_ZERO(&readfds);
  FD_SET(socket_, &readfds);
  
  int ready = select(socket_ + 1, &readfds, nullptr, nullptr, &tv);
  if (ready < 0) {
    return { science::StatusCode::kInternal, "error waiting for data: " + std::string(strerror(errno)) };
  }
  if (ready == 0) {
    return { science::StatusCode::kUnavailable, "no data available" };
  }

  std::vector<uint8_t> buf;
  buf.resize(8192);
  auto rc = recvfrom(socket_, buf.data(), buf.size(), 0, reinterpret_cast<struct sockaddr*>(&saddr), &saddr_len);
  if (rc < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return { science::StatusCode::kUnavailable, "no data available" };
    }
    buf.resize(0);
    return { science::StatusCode::kInternal, "error reading from socket (code: " + std::to_string(rc) + ")" };
  }

  buf.resize(rc);
  return unpack(buf, data, header, bytes_read);
}

auto StreamOut::from_proto(const synapse::NodeConfig& proto, std::shared_ptr<Node>* node) -> science::Status {
  if (!proto.has_stream_out()) {
    return { science::StatusCode::kInvalidArgument, "missing stream_out config" };
  }

  const auto& config = proto.stream_out();
  const auto& label = config.label();

  if (!config.has_udp_unicast()) {
    // Use defaults
    *node = std::make_shared<StreamOut>("", DEFAULT_STREAM_OUT_PORT, label);
    return {};
  }

  const auto& unicast = config.udp_unicast();
  std::string dest_addr = unicast.destination_address();
  uint16_t dest_port = unicast.destination_port();

  if (dest_addr.empty()) {
    dest_addr = get_client_ip();
  }
  if (dest_port == 0) {
    dest_port = DEFAULT_STREAM_OUT_PORT;
  }

  *node = std::make_shared<StreamOut>(dest_addr, dest_port, label);
  return {};
}

auto StreamOut::p_to_proto(synapse::NodeConfig* proto) -> science::Status {
  if (proto == nullptr) {
    return { science::StatusCode::kInvalidArgument, "proto ptr must not be null" };
  }

  synapse::StreamOutConfig* config = proto->mutable_stream_out();
  synapse::UDPUnicastConfig* unicast = config->mutable_udp_unicast();
  unicast->set_destination_address(destination_address_.empty() ? LOCALHOST : destination_address_);
  unicast->set_destination_port(destination_port_);
  config->set_label(label_);

  return {};
}

}  // namespace synapse
