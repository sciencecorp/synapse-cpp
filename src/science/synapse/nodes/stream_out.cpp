#include "science/synapse/nodes/stream_out.h"

namespace synapse {

StreamOut::StreamOut(std::optional<ChannelMask> channel_mask, std::optional<std::string> multicast_group)
  : Node(NodeType::kStreamOut),
    channel_mask_(channel_mask),
    multicast_group_(multicast_group) {}

auto StreamOut::read() const -> std::variant<bool, std::vector<std::byte>> {
  return {};
}

auto StreamOut::p_to_proto(synapse::NodeConfig* proto) -> void {
  synapse::StreamOutConfig* config = proto->mutable_stream_out();

  if (channel_mask_.has_value()) {
    const auto& channels = channel_mask_->channels();
    for (size_t c = 0; c < channels.size(); ++c) {
      if (!channels[c]) {
        continue;
      }
      config->add_ch_mask(c);
    }
  }

  if (multicast_group_.has_value()) {
    config->set_multicast_group(multicast_group_.value());
  }
}

}  // namespace synapse
