#pragma once

#include <future>
#include <memory>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>
#include "science/synapse/api/app.pb.h"
#include "science/synapse/api/device.pb.h"
#include "science/synapse/api/logging.pb.h"
#include "science/synapse/api/node.pb.h"
#include "science/synapse/api/query.pb.h"
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
  virtual auto uri() const -> const std::string& = 0;
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
   * Execute a query on the device.
   *
   * @param request The query request.
   * @param response Output parameter for the query response.
   * @param timeout Optional timeout for the query.
   * @return Status indicating success or failure.
   */
  [[nodiscard]] auto query(const synapse::QueryRequest& request,
                           synapse::QueryResponse* response,
                           std::optional<std::chrono::milliseconds> timeout = std::nullopt) -> science::Status;

  /**
   * Get device logs.
   *
   * @param request Log query parameters (time range, level filter).
   * @param response Output parameter for log entries.
   * @param timeout Optional timeout.
   * @return Status indicating success or failure.
   */
  [[nodiscard]] auto get_logs(const synapse::LogQueryRequest& request,
                               synapse::LogQueryResponse* response,
                               std::optional<std::chrono::milliseconds> timeout = std::nullopt) -> science::Status;

  /**
   * Update device settings.
   *
   * @param request Settings to update.
   * @param response Output parameter for updated settings.
   * @param timeout Optional timeout.
   * @return Status indicating success or failure.
   */
  [[nodiscard]] auto update_settings(const synapse::UpdateDeviceSettingsRequest& request,
                                      synapse::UpdateDeviceSettingsResponse* response,
                                      std::optional<std::chrono::milliseconds> timeout = std::nullopt) -> science::Status;

  /**
   * List installed applications on the device.
   *
   * @param response Output parameter for list of apps.
   * @param timeout Optional timeout.
   * @return Status indicating success or failure.
   */
  [[nodiscard]] auto list_apps(synapse::ListAppsResponse* response,
                                std::optional<std::chrono::milliseconds> timeout = std::nullopt) -> science::Status;

  /**
   * Get the device URI.
   *
   * @return The device URI string.
   */
  [[nodiscard]] auto uri() const -> const std::string&;

 private:
  std::string uri_;
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<synapse::SynapseDevice::Stub> rpc_;

  [[nodiscard]] auto handle_status_response(const synapse::Status& status) -> science::Status;
};

}  // namespace synapse
