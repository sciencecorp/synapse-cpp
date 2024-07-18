#include "science/synapse/channel_mask.h"

namespace synapse {

ChannelMask::ChannelMask(size_t size) : channels_(size, true) {}

ChannelMask::ChannelMask(ChannelMaskBits channels) : channels_(channels) {}

auto ChannelMask::channels() const -> ChannelMaskBits {
  return channels_;
}

}  // namespace synapse
