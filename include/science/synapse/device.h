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
  auto configure(Config* config) -> science::Status;
  /**
   * @see synapse::SynapseDevice::Stub#info
   */
  auto info(synapse::DeviceInfo* info) -> science::Status;
  /**
   * @see synapse::SynapseDevice::Stub#start
   */
  auto start() -> science::Status;
  /**
   * @see synapse::SynapseDevice::Stub#start
   */
  auto stop() -> science::Status;

  /**
   * List the node sockets configured on the device. 
   * 
   * @return std::vector<synapse::NodeSocket> 
   */
  auto sockets() const -> const std::vector<synapse::NodeSocket>&;

  /**
   * List the node sockets configured on the device. 
   * 
   * @return std::vector<synapse::NodeSocket> 
   */
  auto uri() const -> const std::string&;

 private:
  std::string uri_;
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<synapse::SynapseDevice::Stub> rpc_;

  std::vector<synapse::NodeSocket> sockets_;

  auto handle_status_response(const synapse::Status& status) -> science::Status;
};

}  // namespace synapse
