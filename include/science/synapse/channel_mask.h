#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <utility>

#include "science/synapse/api/node.pb.h"

namespace synapse {

class ChannelMask {
 public:
  explicit ChannelMask(size_t size);
  explicit ChannelMask(const std::vector<uint32_t>& channels);
  ChannelMask(std::vector<uint32_t>::const_iterator begin, std::vector<uint32_t>::const_iterator end);

  auto channels() const -> std::vector<uint32_t>;

 private:
  std::vector<uint32_t> channels_;
};

}  // namespace synapse
