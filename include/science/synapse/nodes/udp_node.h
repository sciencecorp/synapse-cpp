#pragma once

#include <netinet/in.h>
#include <string>
#include <vector>

#include "science/scipp/status.h"
#include "science/synapse/node.h"

namespace synapse {

class UdpNode : public Node {
 public:
  explicit UdpNode(const synapse::NodeType& node_type);
  virtual ~UdpNode();

 protected:
  auto addr() const -> std::optional<sockaddr_in>;
  auto init() -> science::Status;
  auto sock() const -> int;
  virtual auto get_host(std::string* host) -> science::Status;

 private:
  int socket_ = 0;
  std::optional<sockaddr_in> addr_;

  auto get_port(uint16_t* port) -> science::Status;
};

}  // namespace synapse
