#pragma once

#include <string>
#include <vector>

#include "science/scipp/status.h"
#include "science/synapse/device_advertisement.h"

namespace synapse {


/**
 * Discover Synapse devices on the network.
 * 
 * Sends a DISCOVERY request to the network and waits for responses.
 * 
 * @param timeout_ms The timeout in milliseconds. Optional.
 * @param discovered A list of discovered devices to populate. Optional.
 * @return science::Status 
 */
auto discover(unsigned int timeout_ms = 3000,
              std::vector<DeviceAdvertisement>* discovered = nullptr) -> science::Status;

auto parse(const std::string& host,
           const std::vector<std::string>& payload,
           DeviceAdvertisement* parsed) -> science::Status;
}
