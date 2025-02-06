#pragma once

#include <netinet/in.h>
#include <memory>
#include <string>
#include <vector>

#include "science/scipp/status.h"
#include "science/synapse/api/nodes/stream_in.pb.h"
#include "science/synapse/nodes/udp_node.h"

namespace synapse {

class StreamIn : public UdpNode {
 public:
  explicit StreamIn(const synapse::DataType& data_type, const std::vector<uint32_t>& shape);

  auto write(const std::vector<std::byte>& in) -> science::Status;

  [[nodiscard]] static auto from_proto(
    const synapse::NodeConfig& proto,
    std::shared_ptr<Node>* node
  ) -> science::Status;

 protected:
  auto p_to_proto(synapse::NodeConfig* proto) -> science::Status override;

 private:
  const synapse::DataType data_type_;
  const std::vector<uint32_t> shape_;
  auto init() -> science::Status;
};

}  // namespace synapse
