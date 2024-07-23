#pragma once

#include <netinet/in.h>
#include <string>
#include <vector>

#include "science/scipp/status.h"
#include "science/synapse/api/nodes/stream_out.pb.h"
#include "science/synapse/nodes/udp_node.h"

namespace synapse {

class StreamOut : public UdpNode {
 public:
  StreamOut(
    const synapse::DataType& data_type,
    const std::vector<uint32_t>& shape,
    std::optional<std::string> multicast_group
  );

  auto read(std::vector<std::byte>* out) -> science::Status;

 protected:
  auto p_to_proto(synapse::NodeConfig* proto) -> void override;

 private:
  const synapse::DataType data_type_;
  const std::vector<uint32_t> shape_;
  const std::optional<std::string> multicast_group_;

  auto init() -> science::Status;
  auto get_host(std::string* host) -> science::Status override;
};

}  // namespace synapse
