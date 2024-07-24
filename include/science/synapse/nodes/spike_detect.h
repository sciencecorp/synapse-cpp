#pragma once

#include <memory>
#include "science/synapse/api/nodes/spike_detect.pb.h"
#include "science/synapse/channel_mask.h"
#include "science/synapse/node.h"

namespace synapse {

class SpikeDetect : public Node {
 public:
  explicit SpikeDetect(
    const synapse::SpikeDetectOptions::SpikeDetectMode& mode,
    uint32_t threshold_uv,
    const ChannelMask& template_uv,
    bool sort
  );

  [[nodiscard]] static auto from_proto(
    const synapse::NodeConfig& proto,
    std::shared_ptr<Node>* node
  ) -> science::Status;


 protected:
  auto p_to_proto(synapse::NodeConfig* proto) -> void override;

 private:
  synapse::SpikeDetectOptions::SpikeDetectMode mode_;
  uint32_t threshold_uv_;
  ChannelMask template_uv_;
  bool sort_;
};

}  // namespace synapse
