#pragma once
#pragma once

#include <memory>
#include <string>
#include <vector>
#include <utility>

#include "science/synapse/api/node.pb.h"

namespace synapse {

using ChannelMaskBits = std::vector<bool>;

class ChannelMask {
 public:
  explicit ChannelMask(size_t size);
  explicit ChannelMask(ChannelMaskBits channels);

  auto channels() const -> ChannelMaskBits;

 private:
  ChannelMaskBits channels_;
};

}  // namespace synapse
