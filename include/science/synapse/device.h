#pragma once

#include <future>
#include <memory>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>
#include "science/synapse/api/node.pb.h"
#include "science/synapse/api/synapse.pb.h"
#include "science/synapse/api/synapse.grpc.pb.h"
#include "science/synapse/config.h"
#include "science/synapse/device_advertisement.h"

namespace synapse {

class IDevice {
 public:
  virtual ~IDevice() = default;
  virtual auto configure(Config* config, std::optional<std::chrono::milliseconds> timeout = std::nullopt) -> science::Status = 0;
  virtual auto info(synapse::DeviceInfo* info, std::optional<std::chrono::milliseconds> timeout = std::nullopt) -> science::Status = 0;
  virtual auto start(std::optional<std::chrono::milliseconds> timeout = std::nullopt) -> science::Status = 0;
  virtual auto stop(std::optional<std::chrono::milliseconds> timeout = std::nullopt) -> science::Status = 0;
  virtual auto sockets() const -> const std::vector<synapse::NodeSocket>& = 0;
  virtual auto uri() const -> const std::string& = 0;
  virtual auto get_logs(
      const std::string& log_level = "INFO",
      std::optional<int64_t> since_ms = std::nullopt,
      std::optional<int64_t> start_time_ns = std::nullopt,
      std::optional<int64_t> end_time_ns = std::nullopt) -> std::vector<std::string> = 0;
  virtual auto tail_logs(
      const std::string& log_level = "INFO",
      std::optional<std::chrono::milliseconds> timeout = std::nullopt) -> std::vector<std::string> = 0;
};

/**
 * A client for a Synapse device.
 * 
 * Use this class to make calls to a Synapse device.
 * The Device must be configured via Config before its signal chain can be run.
 */
class Device : public IDevice {
 public:
  explicit Device(const std::string& uri);

  /**
   * @see synapse::SynapseDevice::Stub#Configure
   */
  [[nodiscard]] auto configure(Config* config, std::optional<std::chrono::milliseconds> timeout = std::nullopt) -> science::Status;
  /**
   * @see synapse::SynapseDevice::Stub#info
   */
  [[nodiscard]] auto info(synapse::DeviceInfo* info, std::optional<std::chrono::milliseconds> timeout = std::nullopt) -> science::Status;
  /**
   * @see synapse::SynapseDevice::Stub#start
   */
  [[nodiscard]] auto start(std::optional<std::chrono::milliseconds> timeout = std::nullopt) -> science::Status;
  /**
   * @see synapse::SynapseDevice::Stub#start
   */
  [[nodiscard]] auto stop(std::optional<std::chrono::milliseconds> timeout = std::nullopt) -> science::Status;

  /**
   * List the node sockets configured on the device. 
   * 
   * @return std::vector<synapse::NodeSocket> 
   */
  [[nodiscard]] auto sockets() const -> const std::vector<synapse::NodeSocket>&;

  /**
   * List the node sockets configured on the device. 
   * 
   * @return std::vector<synapse::NodeSocket> 
   */
  [[nodiscard]] auto uri() const -> const std::string&;

  /**
   * Get the logs from the device.
   * 
   * @param log_level Minimum log level to retrieve (default: INFO)
   * @param since_ms Get logs from this many milliseconds ago (optional)
   * @param start_time_ns Start time in nanoseconds since epoch (optional)
   * @param end_time_ns End time in nanoseconds since epoch (optional)
   * @return std::vector<std::string> containing log entries
   */
  [[nodiscard]] auto get_logs(
      const std::string& log_level = "INFO",
      std::optional<int64_t> since_ms = std::nullopt,
      std::optional<int64_t> start_time_ns = std::nullopt,
      std::optional<int64_t> end_time_ns = std::nullopt) -> std::vector<std::string>;

  /**
   * Tail the logs from the device.
   * 
   * @param log_level Minimum log level to retrieve (default: INFO)
   * @param timeout Timeout for the request (optional)
   * @return std::vector<std::string> containing the most recent log entries
   */
  [[nodiscard]] auto tail_logs(
      const std::string& log_level = "INFO",
      std::optional<std::chrono::milliseconds> timeout = std::nullopt) -> std::vector<std::string>;

 private:
  std::string uri_;
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<synapse::SynapseDevice::Stub> rpc_;

  std::vector<synapse::NodeSocket> sockets_;

  [[nodiscard]] auto handle_status_response(const synapse::Status& status) -> science::Status;

  [[nodiscard]] auto log_level_to_pb(const std::string& level) -> synapse::LogLevel;
};

}  // namespace synapse
