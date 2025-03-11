#include "science/synapse/nodes/broadband_source.h"

namespace synapse {


BroadbandSource::BroadbandSource(
  uint32_t peripheral_id,
  uint32_t bit_width,
  uint32_t sample_rate_hz_,
  float gain,
  const Signal& signal
) : Node(NodeType::kBroadbandSource),
    peripheral_id_(peripheral_id),
    bit_width_(bit_width),
    sample_rate_hz_(sample_rate_hz_),
    gain_(gain),
    signal_(signal) {}

auto BroadbandSource::from_proto(const synapse::NodeConfig& proto, std::shared_ptr<Node>* node) -> science::Status {
  if (!proto.has_broadband_source()) {
    return { science::StatusCode::kInvalidArgument, "missing broadband_source config" };
  }

  const auto& config = proto.broadband_source();
  if (!config.has_signal()) {
    return { science::StatusCode::kInvalidArgument, "missing signal configuration" };
  }

  const auto& signal_proto = config.signal();
  Signal signal;

  auto s = Signal::from_proto(signal_proto, &signal);
  if (!s.ok()) {
    return s;
  }

  *node = std::make_shared<BroadbandSource>(
    config.peripheral_id(),
    config.bit_width(),
    config.sample_rate_hz(),
    config.gain(),
    signal
  );

  return {};
}

auto BroadbandSource::p_to_proto(synapse::NodeConfig* proto) -> science::Status {
  if (proto == nullptr) {
    return { science::StatusCode::kInvalidArgument, "proto ptr must not be null" };
  }

  synapse::BroadbandSourceConfig* config = proto->mutable_broadband_source();

  config->set_peripheral_id(peripheral_id_);
  config->set_bit_width(bit_width_);
  config->set_sample_rate_hz (sample_rate_hz_);
  config->set_gain(gain_);

  auto s = signal_.to_proto(config->mutable_signal());
  std::cout << "BroadbandSource::p_to_proto: " << config->DebugString() << std::endl;

  return s;
}

}  // namespace synapse
