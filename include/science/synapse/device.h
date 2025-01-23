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

/**
 * A client for a Synapse device.
 * 
 * Use this class to make calls to a Synapse device.
 * The Device must be configured via Config before its signal chain can be run.
 */
class Device {
 public:
  explicit Device(const std::string& uri);

  /**
   * @see synapse::SynapseDevice::Stub#Configure
   */
  [[nodiscard]] auto configure(Config* config, std::optional<std::chrono::milliseconds> deadline) -> science::Status;
  /**
   * @see synapse::SynapseDevice::Stub#info
   */
  [[nodiscard]] auto info(synapse::DeviceInfo* info, std::optional<std::chrono::milliseconds> deadline) -> science::Status;
  /**
   * @see synapse::SynapseDevice::Stub#start
   */
  [[nodiscard]] auto start(std::optional<std::chrono::milliseconds> deadline) -> science::Status;
  /**
   * @see synapse::SynapseDevice::Stub#start
   */
  [[nodiscard]] auto stop(std::optional<std::chrono::milliseconds> deadline) -> science::Status;

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

 private:
  std::string uri_;
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<synapse::SynapseDevice::Stub> rpc_;

  std::vector<synapse::NodeSocket> sockets_;

  [[nodiscard]] auto handle_status_response(const synapse::Status& status) -> science::Status;
};

}  // namespace synapse
