#pragma once

#include <memory>
#include <string>
#include <vector>
#include <optional>

#include <zmq.hpp>
#include "science/scipp/status.h"
#include "science/synapse/api/tap.pb.h"

namespace synapse {

/**
 * A client for connecting to Synapse device taps.
 *
 * Taps provide high-throughput data streaming to/from a Synapse device
 * using ZeroMQ. Producer taps emit data (e.g., broadband neural data),
 * while consumer taps accept data (e.g., stimulation commands).
 */
class Tap {
 public:
  /**
   * Create a Tap client for a device.
   *
   * @param device_uri The URI of the Synapse device (e.g., "192.168.1.100:647")
   */
  explicit Tap(const std::string& device_uri);
  ~Tap();

  // Prevent copying
  Tap(const Tap&) = delete;
  Tap& operator=(const Tap&) = delete;

  // Allow moving
  Tap(Tap&&) noexcept;
  Tap& operator=(Tap&&) noexcept;

  /**
   * Query available taps from the device.
   *
   * @return Vector of TapConnection objects describing available taps.
   */
  [[nodiscard]] auto list_taps() -> std::vector<synapse::TapConnection>;

  /**
   * Connect to a tap by name.
   *
   * @param tap_name The name of the tap to connect to.
   * @return Status indicating success or failure.
   */
  [[nodiscard]] auto connect(const std::string& tap_name) -> science::Status;

  /**
   * Disconnect from the current tap.
   */
  void disconnect();

  /**
   * Check if currently connected to a tap.
   *
   * @return true if connected, false otherwise.
   */
  [[nodiscard]] auto is_connected() const -> bool;

  /**
   * Get the currently connected tap info.
   *
   * @return The connected TapConnection, or nullopt if not connected.
   */
  [[nodiscard]] auto connected_tap() const -> std::optional<synapse::TapConnection>;

  /**
   * Read a single message from the tap (blocking with timeout).
   *
   * Only valid for producer taps (TAP_TYPE_PRODUCER).
   *
   * @param out Output buffer for the received data.
   * @param timeout_ms Timeout in milliseconds (default: 1000).
   * @return Status indicating success, timeout, or error.
   */
  [[nodiscard]] auto read(std::vector<uint8_t>* out, int timeout_ms = 1000) -> science::Status;

  /**
   * Send data to the tap.
   *
   * Only valid for consumer taps (TAP_TYPE_CONSUMER).
   *
   * @param data The data to send.
   * @return Status indicating success or failure.
   */
  [[nodiscard]] auto send(const std::vector<uint8_t>& data) -> science::Status;

  /**
   * Read multiple messages in a batch (non-blocking).
   *
   * @param out Output vector for received messages.
   * @param max_messages Maximum number of messages to read.
   * @param timeout_ms Timeout for the entire batch operation.
   * @return Number of messages read.
   */
  [[nodiscard]] auto read_batch(std::vector<std::vector<uint8_t>>* out,
                                 size_t max_messages,
                                 int timeout_ms = 100) -> size_t;

 private:
  std::string device_uri_;
  std::unique_ptr<zmq::context_t> zmq_context_;
  std::unique_ptr<zmq::socket_t> zmq_socket_;
  std::optional<synapse::TapConnection> connected_tap_;

  void cleanup();
};

}  // namespace synapse
