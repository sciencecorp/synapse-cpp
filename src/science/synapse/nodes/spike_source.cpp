#include "science/synapse/nodes/spike_source.h"

namespace synapse {

SpikeSource::SpikeSource(
  uint32_t peripheral_id,
  uint32_t sample_rate_hz,
  float spike_window_ms,
  float gain,
  float threshold_uV,
  const Electrodes& electrodes_config
) : Node(NodeType::kSpikeSource),
    peripheral_id_(peripheral_id),
    sample_rate_hz_(sample_rate_hz),
    spike_window_ms_(spike_window_ms),
    gain_(gain),
    threshold_uV_(threshold_uV),
    electrodes_config_(electrodes_config) {}

auto SpikeSource::from_proto(const synapse::NodeConfig& proto, std::shared_ptr<Node>* node) -> science::Status {
  if (!proto.has_spike_source()) {
    return { science::StatusCode::kInvalidArgument, "missing spike_source config" };
  }

  const auto& config = proto.spike_source();
  if (!config.has_electrodes()) {
    return { science::StatusCode::kInvalidArgument, "missing electrodes configuration" };
  }

  const auto& electrodes_proto = config.electrodes();
  Electrodes electrodes;

  auto s = Electrodes::from_proto(electrodes_proto, &electrodes);
  if (!s.ok()) {
    return s;
  }

  *node = std::make_shared<SpikeSource>(
    config.peripheral_id(),
    config.sample_rate_hz(),
    config.spike_window_ms(),
    config.gain(),
    config.threshold_uv(),
    electrodes
  );

  return {};
}

auto SpikeSource::p_to_proto(synapse::NodeConfig* proto) -> science::Status {
  if (proto == nullptr) {
    return { science::StatusCode::kInvalidArgument, "proto ptr must not be null" };
  }

  synapse::SpikeSourceConfig* config = proto->mutable_spike_source();

  config->set_peripheral_id(peripheral_id_);
  config->set_sample_rate_hz (sample_rate_hz_);
  config->set_spike_window_ms(spike_window_ms_);
  config->set_gain(gain_);
  config->set_threshold_uv(threshold_uV_);

  auto s = electrodes_config_.to_proto(config->mutable_electrodes());

  return s;
}

}  // namespace synapse
