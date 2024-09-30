#include "science/synapse/util/discover.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

namespace synapse {

using science::Status;
using science::StatusCode;

const char BROADCAST_SERVICE[] = "SYN";
const char BROADCAST_ADDR[] = "224.0.0.245";
const uint16_t BROADCAST_PORT = 6470;
const uint16_t BROADCAST_TIMEOUT_SEC = 3;

auto addr(const std::string& host, uint16_t port) -> sockaddr_in {
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  inet_pton(AF_INET, host.c_str(), &addr.sin_addr);
  return addr;
}

auto parse(const std::string& host,
           const std::vector<std::string>& payload,
           DeviceAdvertisement* parsed) -> science::Status {
  if (payload.size() < 5) {
    return { StatusCode::kInvalidArgument, "invalid response from server" };
  }

  std::string cmd = payload[0];
  if (cmd != "ID") {
    return {
      StatusCode::kInvalidArgument,
      "invalid response from server (expected ID, got {" + cmd + "})"
    };
  }

  uint64_t port = 0;
  try {
    port = std::stoi(payload[3]);
  } catch (const std::exception& e) {
    return { StatusCode::kInvalidArgument, "invalid port in response from server" };
  }

  if (port < 1 || port > 65535) {
    return { StatusCode::kInvalidArgument, "invalid port in response from server" };
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
  const std::string host = BROADCAST_ADDR;
  const uint16_t port = BROADCAST_PORT;

  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    return { StatusCode::kInternal, "error creating socket (code: " + std::to_string(sock) + ")" };
  }

  int ttl = 3;
  auto rc = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
  if (rc < 0) {
    return {
      StatusCode::kInternal,
      "error configuring socket options (code: " + std::to_string(rc) + ")"
    };
  }

  auto saddr = addr(host, port);
  std::string payload = "DISCOVER";

  rc = sendto(sock, payload.c_str(), payload.size(), 0, reinterpret_cast<sockaddr*>(&saddr), sizeof(saddr));
  if (rc < 0) {
    return { StatusCode::kInternal, "error sending data to socket (code: " + std::to_string(rc) + ")" };
  }

  fd_set fbuffer;
  char buffer[1024];
  timeval timeout = {
    timeout_ms ? timeout_ms / 1000 : BROADCAST_TIMEOUT_SEC,
    0
  };

  if (discovered != nullptr) {
    discovered->clear();
  }

  while (true) {
    FD_ZERO(&fbuffer);
    FD_SET(sock, &fbuffer);

    int rc = select(sock + 1, &fbuffer, NULL, NULL, &timeout);
    if (rc == 0) {
      break;

    } else if (rc < 0) {
      break;

    } else if (FD_ISSET(sock, &fbuffer)) {
      socklen_t saddr_len = sizeof(saddr);
      int recv_len = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, reinterpret_cast<sockaddr*>(&saddr), &saddr_len);
      if (recv_len < 0) {
        continue;
      }

      buffer[recv_len] = '\0';
      std::istringstream response(buffer);
      std::vector<std::string> tokens;
      std::string token;
      while (response >> token) {
        tokens.push_back(token);
      }

      DeviceAdvertisement res;
      auto s = parse(host, tokens, &res);
      if (!s.ok()) {
        break;
      }

      if (!validate_capability(res.capability)) {
        continue;
      }

      if (discovered != nullptr) {
        discovered->push_back(res);
      }
    }
  }

  return {};
}

}  // namespace synapse
