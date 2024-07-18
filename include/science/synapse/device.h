#pragma once

#include <future>
#include <memory>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>
#include "science/synapse/api/node.pb.h"
#include "science/synapse/api/synapse.pb.h"
#include "science/synapse/api/synapse.grpc.pb.h"
#include "science/synapse/device_advertisement.h"
#include "science/synapse/config.h"

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
  auto configure(Config* config) -> bool;
  /**
   * @see synapse::SynapseDevice::Stub#info
   */
  auto info() -> std::optional<synapse::DeviceInfo>;
  /**
   * @see synapse::SynapseDevice::Stub#start
   */
  auto start() -> bool;
  /**
   * @see synapse::SynapseDevice::Stub#start
   */
  auto stop() -> bool;

 private:
  std::string uri_;
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<synapse::SynapseDevice::Stub> rpc_;

  std::vector<synapse::NodeSocket> sockets_;

  auto handle_status_response(const synapse::Status& status) -> bool;
};

}  // namespace synapse
