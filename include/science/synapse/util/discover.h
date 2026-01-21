#pragma once

#include <functional>
#include <string>
#include <vector>

#include "science/synapse/status.h"
#include "science/synapse/device_advertisement.h"

namespace synapse {

/**
 * Discover Synapse devices on the network.
 *
 * Listens for device broadcast announcements on UDP port 6470.
 * Devices periodically broadcast "ID serial capability port name" messages.
 *
 * @param timeout_ms The timeout in milliseconds (default: 10000ms).
 * @param discovered A vector to populate with discovered devices. Optional.
 * @return science::Status indicating success or failure.
 */
auto discover(unsigned int timeout_ms = 10000,
              std::vector<DeviceAdvertisement>* discovered = nullptr) -> science::Status;

/**
 * Discover devices with a callback for each device found.
 *
 * This allows processing devices as they're discovered rather than
 * waiting for the full timeout.
 *
 * @param callback Called for each device discovered. Return false to stop discovery.
 * @param timeout_ms The timeout in milliseconds (default: 10000ms).
 * @return science::Status indicating success or failure.
 */
auto discover_iter(std::function<bool(const DeviceAdvertisement&)> callback,
                   unsigned int timeout_ms = 10000) -> science::Status;

/**
 * Parse a device advertisement message.
 *
 * @param host The IP address of the sender.
 * @param payload The parsed tokens from the message.
 * @param parsed Output parameter for the parsed advertisement.
 * @return science::Status indicating success or failure.
 */
auto parse(const std::string& host,
           const std::vector<std::string>& payload,
           DeviceAdvertisement* parsed) -> science::Status;

}  // namespace synapse
