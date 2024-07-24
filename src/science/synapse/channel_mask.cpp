#include "science/synapse/channel_mask.h"

namespace synapse {

ChannelMask::ChannelMask(size_t size) : channels_(size, true) {}

ChannelMask::ChannelMask(std::vector<uint32_t> channels) : channels_(channels) {}

ChannelMask::ChannelMask(std::vector<uint32_t>::const_iterator begin, std::vector<uint32_t>::const_iterator end)
  : channels_(begin, end) {}

auto ChannelMask::channels() const -> std::vector<uint32_t> {
  return channels_;
}

}  // namespace synapse
