#pragma once

#include <string>
#include <vector>

#include "science/synapse/api/nodes/stream_in.pb.h"
#include "science/synapse/node.h"

namespace synapse {

class StreamIn : public Node {
 public:
  explicit StreamIn(std::optional<std::string> multicast_group);

  auto write(std::vector<std::byte>) const -> bool;

 protected:
  auto get_addr() const -> std::optional<std::string>;
  auto p_to_proto(synapse::NodeConfig* proto) -> void override;

 private:
  std::optional<std::string> multicast_group_;

  int socket_;
};

}  // namespace synapse
