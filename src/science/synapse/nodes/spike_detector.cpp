#include "science/synapse/nodes/spike_detector.h"

namespace synapse {

SpikeDetector::SpikeDetector()
  : Node(NodeType::kSpikeDetector),
    mode_(Mode::Thresholder),
    threshold_uv_(0),
    samples_per_spike_(0) {}

auto SpikeDetector::create_thresholder(uint32_t threshold_uv, uint32_t samples_per_spike)
    -> std::shared_ptr<SpikeDetector> {
  auto detector = std::shared_ptr<SpikeDetector>(new SpikeDetector());
  detector->mode_ = Mode::Thresholder;
  detector->threshold_uv_ = threshold_uv;
  detector->samples_per_spike_ = samples_per_spike;
  return detector;
}

auto SpikeDetector::create_template_matcher(const std::vector<uint32_t>& template_uv, uint32_t samples_per_spike)
    -> std::shared_ptr<SpikeDetector> {
  auto detector = std::shared_ptr<SpikeDetector>(new SpikeDetector());
  detector->mode_ = Mode::TemplateMatcher;
  detector->template_uv_ = template_uv;
  detector->samples_per_spike_ = samples_per_spike;
  return detector;
}

auto SpikeDetector::from_proto(const synapse::NodeConfig& proto, std::shared_ptr<Node>* node) -> science::Status {
  if (!proto.has_spike_detector()) {
    return { science::StatusCode::kInvalidArgument, "missing spike_detector config" };
  }

  const auto& config = proto.spike_detector();

  if (config.has_thresholder()) {
    *node = create_thresholder(config.thresholder().threshold_uv(), config.samples_per_spike());
  } else if (config.has_template_matcher()) {
    const auto& tm = config.template_matcher();
    std::vector<uint32_t> template_uv(tm.template_uv().begin(), tm.template_uv().end());
    *node = create_template_matcher(template_uv, config.samples_per_spike());
  } else {
    return { science::StatusCode::kInvalidArgument, "spike_detector config must have thresholder or template_matcher" };
  }

  return {};
}

auto SpikeDetector::p_to_proto(synapse::NodeConfig* proto) -> science::Status {
  if (proto == nullptr) {
    return { science::StatusCode::kInvalidArgument, "proto ptr must not be null" };
  }

  synapse::SpikeDetectorConfig* config = proto->mutable_spike_detector();
  config->set_samples_per_spike(samples_per_spike_);

  if (mode_ == Mode::Thresholder) {
    config->mutable_thresholder()->set_threshold_uv(threshold_uv_);
  } else {
    auto* tm = config->mutable_template_matcher();
    for (auto val : template_uv_) {
      tm->add_template_uv(val);
    }
  }

  return {};
}

}  // namespace synapse
