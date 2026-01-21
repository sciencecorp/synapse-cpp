#include "science/synapse/tap.h"
#include "science/synapse/device.h"
#include "science/synapse/api/query.pb.h"

#include <cerrno>
#include <cstring>
#include <regex>

namespace synapse {

Tap::Tap(const std::string& device_uri)
    : device_uri_(device_uri),
      zmq_context_(nullptr),
      zmq_socket_(nullptr),
      connected_tap_(std::nullopt) {}

Tap::~Tap() {
  cleanup();
}

Tap::Tap(Tap&& other) noexcept
    : device_uri_(std::move(other.device_uri_)),
      zmq_context_(std::move(other.zmq_context_)),
      zmq_socket_(std::move(other.zmq_socket_)),
      connected_tap_(std::move(other.connected_tap_)) {}

Tap& Tap::operator=(Tap&& other) noexcept {
  if (this != &other) {
    cleanup();
    device_uri_ = std::move(other.device_uri_);
    zmq_context_ = std::move(other.zmq_context_);
    zmq_socket_ = std::move(other.zmq_socket_);
    connected_tap_ = std::move(other.connected_tap_);
  }
  return *this;
}

auto Tap::list_taps() -> std::vector<synapse::TapConnection> {
  Device device(device_uri_);

  synapse::QueryRequest request;
  request.set_query_type(synapse::QueryRequest::kListTaps);
  request.mutable_list_taps_query();

  synapse::QueryResponse response;
  auto status = device.query(request, &response);

  if (!status.ok()) {
    return {};
  }

  if (response.status().code() != synapse::StatusCode::kOk) {
    return {};
  }

  std::vector<synapse::TapConnection> taps;
  for (const auto& tap : response.list_taps_response().taps()) {
    taps.push_back(tap);
  }

  return taps;
}

auto Tap::connect(const std::string& tap_name) -> science::Status {
  auto taps = list_taps();

  // Find the tap with the specified name
  const synapse::TapConnection* selected_tap = nullptr;
  for (const auto& tap : taps) {
    if (tap.name() == tap_name) {
      selected_tap = &tap;
      break;
    }
  }

  if (!selected_tap) {
    return {science::StatusCode::kNotFound, "Tap '" + tap_name + "' not found"};
  }

  // Initialize ZMQ context
  zmq_context_ = std::make_unique<zmq::context_t>(1);

  // Create appropriate socket type based on tap type
  if (selected_tap->tap_type() == synapse::TapType::TAP_TYPE_CONSUMER) {
    // For consumer taps, we need to publish data TO the tap
    zmq_socket_ = std::make_unique<zmq::socket_t>(*zmq_context_, zmq::socket_type::pub);
  } else {
    // For producer taps (or unspecified), we need to subscribe and listen FROM the tap
    zmq_socket_ = std::make_unique<zmq::socket_t>(*zmq_context_, zmq::socket_type::sub);
  }

  // Optimize ZMQ for high-throughput data
#ifdef _WIN32
  // Use smaller buffer sizes for Windows
  zmq_socket_->set(zmq::sockopt::rcvbuf, 2 * 1024 * 1024);  // 2MB
  zmq_socket_->set(zmq::sockopt::rcvhwm, 5000);
#else
  // Linux/macOS can handle larger buffers
  zmq_socket_->set(zmq::sockopt::rcvbuf, 16 * 1024 * 1024);  // 16MB
  zmq_socket_->set(zmq::sockopt::rcvhwm, 10000);
#endif

  // Set TCP keepalive for connection stability
  zmq_socket_->set(zmq::sockopt::tcp_keepalive, 1);
  zmq_socket_->set(zmq::sockopt::tcp_keepalive_idle, 60);

  // Build the endpoint URL, replacing the host with our device URI
  std::string endpoint = selected_tap->endpoint();
  if (endpoint.find("://") != std::string::npos) {
    // Extract protocol and port from the endpoint
    std::regex endpoint_regex("([^:]+)://[^:]+:(\\d+)");
    std::smatch match;
    if (std::regex_match(endpoint, match, endpoint_regex)) {
      std::string protocol = match[1].str();
      std::string port = match[2].str();

      // Extract host from device URI (strip port if present)
      std::string host = device_uri_;
      auto colon_pos = host.find(':');
      if (colon_pos != std::string::npos) {
        host = host.substr(0, colon_pos);
      }

      endpoint = protocol + "://" + host + ":" + port;
    }
  }

  try {
    zmq_socket_->connect(endpoint);

    // Only set subscription for subscriber sockets
    if (selected_tap->tap_type() != synapse::TapType::TAP_TYPE_CONSUMER) {
      zmq_socket_->set(zmq::sockopt::subscribe, "");
    }

    connected_tap_ = *selected_tap;
    return {};
  } catch (const zmq::error_t& e) {
    cleanup();
    return {science::StatusCode::kInternal, "Failed to connect to tap: " + std::string(e.what())};
  }
}

void Tap::disconnect() {
  cleanup();
}

auto Tap::is_connected() const -> bool {
  return connected_tap_.has_value() && zmq_socket_ != nullptr;
}

auto Tap::connected_tap() const -> std::optional<synapse::TapConnection> {
  return connected_tap_;
}

auto Tap::read(std::vector<uint8_t>* out, int timeout_ms) -> science::Status {
  if (!is_connected()) {
    return {science::StatusCode::kFailedPrecondition, "Not connected to any tap"};
  }

  if (connected_tap_->tap_type() == synapse::TapType::TAP_TYPE_CONSUMER) {
    return {science::StatusCode::kInvalidArgument, "Cannot read from consumer tap"};
  }

  if (out == nullptr) {
    return {science::StatusCode::kInvalidArgument, "Output buffer cannot be null"};
  }

  try {
    zmq_socket_->set(zmq::sockopt::rcvtimeo, timeout_ms);

    zmq::message_t message;
    auto result = zmq_socket_->recv(message);

    if (!result.has_value()) {
      return {science::StatusCode::kDeadlineExceeded, "Timeout waiting for data"};
    }

    out->resize(message.size());
    std::memcpy(out->data(), message.data(), message.size());

    return {};
  } catch (const zmq::error_t& e) {
    if (e.num() == EAGAIN) {
      return {science::StatusCode::kDeadlineExceeded, "Timeout waiting for data"};
    }
    return {science::StatusCode::kInternal, "Error receiving message: " + std::string(e.what())};
  }
}

auto Tap::send(const std::vector<uint8_t>& data) -> science::Status {
  if (!is_connected()) {
    return {science::StatusCode::kFailedPrecondition, "Not connected to any tap"};
  }

  if (connected_tap_->tap_type() != synapse::TapType::TAP_TYPE_CONSUMER) {
    return {science::StatusCode::kInvalidArgument, "Can only send to consumer tap"};
  }

  try {
    zmq::message_t message(data.data(), data.size());
    auto result = zmq_socket_->send(message, zmq::send_flags::none);

    if (!result.has_value()) {
      return {science::StatusCode::kInternal, "Failed to send message"};
    }

    return {};
  } catch (const zmq::error_t& e) {
    return {science::StatusCode::kInternal, "Error sending message: " + std::string(e.what())};
  }
}

auto Tap::read_batch(std::vector<std::vector<uint8_t>>* out,
                      size_t max_messages,
                      int timeout_ms) -> size_t {
  if (!is_connected() || out == nullptr) {
    return 0;
  }

  if (connected_tap_->tap_type() == synapse::TapType::TAP_TYPE_CONSUMER) {
    return 0;
  }

  out->clear();
  out->reserve(max_messages);

  zmq_socket_->set(zmq::sockopt::rcvtimeo, timeout_ms);

  try {
    while (out->size() < max_messages) {
      zmq::message_t message;
      auto result = zmq_socket_->recv(message, zmq::recv_flags::dontwait);

      if (!result.has_value()) {
        // No more messages available right now
        break;
      }

      std::vector<uint8_t> data(message.size());
      std::memcpy(data.data(), message.data(), message.size());
      out->push_back(std::move(data));
    }
  } catch (const zmq::error_t&) {
    // Ignore errors in batch mode
  }

  return out->size();
}

void Tap::cleanup() {
  if (zmq_socket_) {
    zmq_socket_->close();
    zmq_socket_.reset();
  }

  if (zmq_context_) {
    zmq_context_->close();
    zmq_context_.reset();
  }

  connected_tap_.reset();
}

}  // namespace synapse
