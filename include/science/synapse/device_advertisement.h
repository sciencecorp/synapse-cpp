#pragma once

#include <cstdint>
#include <string>

namespace synapse {

struct DeviceAdvertisement {
  std::string serial;
  std::string capability;
  std::string name;
  std::string host;
  uint16_t port;
};

}  // namespace synapse
