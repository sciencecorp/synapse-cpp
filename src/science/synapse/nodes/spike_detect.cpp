#include "science/synapse/nodes/spike_detect.h"

namespace synapse {

SpikeDetect::SpikeDetect(
  const synapse::SpikeDetectConfig::SpikeDetectMode& mode,
  uint32_t threshold_uv,
  const ChannelMask& template_uv,
  bool sort,
  uint32_t bin_size_ms
) : Node(NodeType::kSpikeDetect),
    mode_(mode),
    threshold_uv_(threshold_uv),
    template_uv_(template_uv),
    sort_(sort),
    bin_size_ms_(bin_size_ms) {}

auto SpikeDetect::from_proto(const synapse::NodeConfig& proto, std::shared_ptr<Node>* node) -> science::Status {
  if (!proto.has_spike_detect()) {
    return { science::StatusCode::kInvalidArgument, "missing spike_detect config" };
  }

  const auto& config = proto.spike_detect();
  ChannelMask template_uv(std::vector<uint32_t>(config.template_uv().begin(), config.template_uv().end()));

  *node = std::make_shared<SpikeDetect>(
    config.mode(),
    config.threshold_uv(),
    template_uv,
    config.sort(),
    config.bin_size_ms()
  );

  return {};
}

auto SpikeDetect::p_to_proto(synapse::NodeConfig* proto) -> science::Status {
  if (proto == nullptr) {
    return { science::StatusCode::kInvalidArgument, "proto ptr must not be null" };
  }

  synapse::SpikeDetectConfig* config = proto->mutable_spike_detect();

  const auto& temp = template_uv_.channels();
  for (size_t i = 0; i < temp.size(); ++i) {
    if (!temp[i]) {
      continue;
    }
    config->add_template_uv(i);
  }

  config->set_mode(mode_);
  config->set_threshold_uv(threshold_uv_);
  config->set_sort(sort_);
  config->set_bin_size_ms(bin_size_ms_);

  return {};
}

}  // namespace synapse
