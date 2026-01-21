#include "science/synapse/util/discover.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>
#include <chrono>
#include <functional>
#include <set>
#include <sstream>
#include <vector>

namespace synapse {

using science::Status;
using science::StatusCode;

const char BROADCAST_SERVICE[] = "SYN";
const uint16_t BROADCAST_PORT = 6470;

auto parse(const std::string& host,
           const std::vector<std::string>& payload,
           DeviceAdvertisement* parsed) -> science::Status {
  if (payload.size() < 5) {
    return {StatusCode::kInvalidArgument, "invalid response from server"};
  }

  std::string cmd = payload[0];
  if (cmd != "ID") {
    return {
        StatusCode::kInvalidArgument,
        "invalid response from server (expected ID, got {" + cmd + "})"};
  }

  uint64_t port = 0;
  try {
    port = std::stoi(payload[3]);
  } catch (const std::exception& e) {
    return {StatusCode::kInvalidArgument, "invalid port in response from server"};
  }

  if (port < 1 || port > 65535) {
    return {StatusCode::kInvalidArgument, "invalid port in response from server"};
  }

  parsed->serial = payload[1];
  parsed->capability = payload[2];
  parsed->name = payload[4];
  parsed->host = host;
  parsed->port = static_cast<uint16_t>(port);

  return {};
}

auto validate_capability(const std::string& capability_str) -> bool {
  auto i = capability_str.find_first_of("0123456789");
  if (i == std::string::npos) {
    return false;
  }

  std::string cap_svc = capability_str.substr(0, i);
  if (cap_svc.empty()) {
    return false;
  }

  return cap_svc == BROADCAST_SERVICE;
}

auto discover(unsigned int timeout_ms,
              std::vector<DeviceAdvertisement>* discovered) -> science::Status {
  // Default timeout: 10 seconds (matching Python)
  if (timeout_ms == 0) {
    timeout_ms = 10000;
  }

  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    return {StatusCode::kInternal,
            "error creating socket (code: " + std::to_string(sock) + ")"};
  }

  // Enable address reuse
  int reuse = 1;
  auto rc = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
  if (rc < 0) {
    close(sock);
    return {StatusCode::kInternal,
            "error configuring SO_REUSEADDR (code: " + std::to_string(rc) + ")"};
  }

#ifdef SO_REUSEPORT
  rc = setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));
  if (rc < 0) {
    close(sock);
    return {StatusCode::kInternal,
            "error configuring SO_REUSEPORT (code: " + std::to_string(rc) + ")"};
  }
#endif

  // Bind to broadcast port to listen for device announcements
  sockaddr_in bind_addr;
  bind_addr.sin_family = AF_INET;
  bind_addr.sin_port = htons(BROADCAST_PORT);
  bind_addr.sin_addr.s_addr = INADDR_ANY;

  rc = bind(sock, reinterpret_cast<sockaddr*>(&bind_addr), sizeof(bind_addr));
  if (rc < 0) {
    close(sock);
    return {StatusCode::kInternal,
            "error binding socket (code: " + std::to_string(rc) + ")"};
  }

  if (discovered != nullptr) {
    discovered->clear();
  }

  auto start_time = std::chrono::steady_clock::now();
  char buffer[1024];

  while (true) {
    // Check if we've exceeded the timeout
    auto now = std::chrono::steady_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
    if (elapsed_ms >= timeout_ms) {
      break;
    }

    // Calculate remaining time for select
    auto remaining_ms = timeout_ms - elapsed_ms;
    timeval timeout = {
        static_cast<long>(remaining_ms / 1000),
        static_cast<long>((remaining_ms % 1000) * 1000)
    };

    fd_set fbuffer;
    FD_ZERO(&fbuffer);
    FD_SET(sock, &fbuffer);

    int rc = select(sock + 1, &fbuffer, nullptr, nullptr, &timeout);
    if (rc == 0) {
      // Timeout on this iteration, check overall timeout
      continue;
    } else if (rc < 0) {
      break;
    }

    if (FD_ISSET(sock, &fbuffer)) {
      sockaddr_in sender_addr;
      socklen_t sender_len = sizeof(sender_addr);
      int recv_len = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                               reinterpret_cast<sockaddr*>(&sender_addr), &sender_len);
      if (recv_len < 0) {
        continue;
      }

      buffer[recv_len] = '\0';

      // Parse the response: "ID serial capability port name"
      std::istringstream response(buffer);
      std::vector<std::string> tokens;
      std::string token;
      while (response >> token) {
        tokens.push_back(token);
      }

      if (tokens.empty() || tokens[0] != "ID") {
        continue;
      }

      // Get sender's IP address
      char sender_ip[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &sender_addr.sin_addr, sender_ip, INET_ADDRSTRLEN);

      DeviceAdvertisement device;
      auto s = parse(sender_ip, tokens, &device);
      if (!s.ok()) {
        continue;
      }

      if (!validate_capability(device.capability)) {
        continue;
      }

      // Check for duplicates
      if (discovered != nullptr) {
        bool is_duplicate = false;
        for (const auto& existing : *discovered) {
          if (existing.serial == device.serial && existing.host == device.host) {
            is_duplicate = true;
            break;
          }
        }
        if (!is_duplicate) {
          discovered->push_back(device);
        }
      }
    }
  }

  close(sock);
  return {};
}

auto discover_iter(std::function<bool(const DeviceAdvertisement&)> callback,
                   unsigned int timeout_ms) -> science::Status {
  if (!callback) {
    return {StatusCode::kInvalidArgument, "callback must not be null"};
  }

  // Default timeout: 10 seconds (matching Python)
  if (timeout_ms == 0) {
    timeout_ms = 10000;
  }

  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    return {StatusCode::kInternal,
            "error creating socket (code: " + std::to_string(sock) + ")"};
  }

  // Enable address reuse
  int reuse = 1;
  auto rc = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
  if (rc < 0) {
    close(sock);
    return {StatusCode::kInternal,
            "error configuring SO_REUSEADDR (code: " + std::to_string(rc) + ")"};
  }

#ifdef SO_REUSEPORT
  rc = setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));
  if (rc < 0) {
    close(sock);
    return {StatusCode::kInternal,
            "error configuring SO_REUSEPORT (code: " + std::to_string(rc) + ")"};
  }
#endif

  // Bind to broadcast port to listen for device announcements
  sockaddr_in bind_addr;
  bind_addr.sin_family = AF_INET;
  bind_addr.sin_port = htons(BROADCAST_PORT);
  bind_addr.sin_addr.s_addr = INADDR_ANY;

  rc = bind(sock, reinterpret_cast<sockaddr*>(&bind_addr), sizeof(bind_addr));
  if (rc < 0) {
    close(sock);
    return {StatusCode::kInternal,
            "error binding socket (code: " + std::to_string(rc) + ")"};
  }

  // Track seen devices to avoid duplicate callbacks
  std::set<std::string> seen_devices;

  auto start_time = std::chrono::steady_clock::now();
  char buffer[1024];

  while (true) {
    // Check if we've exceeded the timeout
    auto now = std::chrono::steady_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
    if (elapsed_ms >= timeout_ms) {
      break;
    }

    // Calculate remaining time for select (use 1 second intervals for responsiveness)
    auto remaining_ms = std::min(static_cast<long long>(1000),
                                  static_cast<long long>(timeout_ms - elapsed_ms));
    timeval timeout = {
        static_cast<long>(remaining_ms / 1000),
        static_cast<long>((remaining_ms % 1000) * 1000)
    };

    fd_set fbuffer;
    FD_ZERO(&fbuffer);
    FD_SET(sock, &fbuffer);

    int rc = select(sock + 1, &fbuffer, nullptr, nullptr, &timeout);
    if (rc == 0) {
      continue;
    } else if (rc < 0) {
      break;
    }

    if (FD_ISSET(sock, &fbuffer)) {
      sockaddr_in sender_addr;
      socklen_t sender_len = sizeof(sender_addr);
      int recv_len = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                               reinterpret_cast<sockaddr*>(&sender_addr), &sender_len);
      if (recv_len < 0) {
        continue;
      }

      buffer[recv_len] = '\0';

      // Parse the response: "ID serial capability port name"
      std::istringstream response(buffer);
      std::vector<std::string> tokens;
      std::string token;
      while (response >> token) {
        tokens.push_back(token);
      }

      if (tokens.empty() || tokens[0] != "ID") {
        continue;
      }

      // Get sender's IP address
      char sender_ip[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &sender_addr.sin_addr, sender_ip, INET_ADDRSTRLEN);

      DeviceAdvertisement device;
      auto s = parse(sender_ip, tokens, &device);
      if (!s.ok()) {
        continue;
      }

      if (!validate_capability(device.capability)) {
        continue;
      }

      // Check for duplicates using serial + host as key
      std::string device_key = device.serial + ":" + device.host;
      if (seen_devices.find(device_key) != seen_devices.end()) {
        continue;
      }
      seen_devices.insert(device_key);

      // Call the callback; if it returns false, stop discovery
      if (!callback(device)) {
        break;
      }
    }
  }

  close(sock);
  return {};
}

}  // namespace synapse
