#pragma once

#include <string>
#include <vector>

#include "science/synapse/api/nodes/stream_out.pb.h"
#include "science/synapse/channel_mask.h"
#include "science/synapse/node.h"

namespace synapse {

class StreamOut : public Node {
 public:
  StreamOut(std::optional<ChannelMask> channel_mask, std::optional<std::string> multicast_group);

  auto read() const -> std::variant<bool, std::vector<std::byte>>;

 protected:
  auto get_addr() const -> std::optional<std::string>;
  auto p_to_proto(synapse::NodeConfig* proto) -> void override;

 private:
  std::optional<ChannelMask> channel_mask_;
  std::optional<std::string> multicast_group_;

  int socket_;
};

}  // namespace synapse
