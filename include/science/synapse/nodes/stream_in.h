#pragma once

#include <netinet/in.h>
#include <string>
#include <vector>

#include "science/scipp/status.h"
#include "science/synapse/api/nodes/stream_in.pb.h"
#include "science/synapse/nodes/udp_node.h"

namespace synapse {

class StreamIn : public UdpNode {
 public:
  StreamIn();

  auto write(const std::vector<std::byte>& in) -> science::Status;

 protected:
  auto p_to_proto(synapse::NodeConfig* proto) -> void override;

 private:
  auto init() -> science::Status;
};

}  // namespace synapse
