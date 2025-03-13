#pragma once

#include <netinet/in.h>
#include <memory>
#include <string>
#include <vector>

#include "science/libndtp/types.h"
#include "science/scipp/status.h"
#include "science/synapse/api/nodes/stream_out.pb.h"
#include "science/synapse/nodes/udp_node.h"

namespace synapse {

class StreamOut : public UdpNode {
 public:
  StreamOut(const std::string& label, const std::string& multicast_group);

  auto read(science::libndtp::SynapseData* out, science::libndtp::NDTPHeader* header = nullptr, size_t* bytes_read = nullptr) -> science::Status;

  [[nodiscard]] static auto from_proto(
    const synapse::NodeConfig& proto,
    std::shared_ptr<Node>* node
  ) -> science::Status;

 protected:
  auto p_to_proto(synapse::NodeConfig* proto) -> science::Status override;

 private:
  const std::string label_;
  const std::string multicast_group_;

  auto init() -> science::Status;
  auto get_host(std::string* host) -> science::Status override;
};

}  // namespace synapse
