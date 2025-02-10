#include "science/synapse/signal_config.h"

namespace synapse {

auto Electrodes::to_proto(synapse::ElectrodeConfig* proto) -> science::Status {
  if (proto == nullptr) {
    return { science::StatusCode::kInvalidArgument, "proto ptr must not be null" };
  }

  for (const auto& channel : channels) {
    channel.to_proto(proto->add_channels());
  }
  proto->set_low_cutoff_hz(low_cutoff_hz);
  proto->set_high_cutoff_hz(high_cutoff_hz);
  return {};
}

auto Electrodes::from_proto(const synapse::ElectrodeConfig& proto, Electrodes* config) -> science::Status {
  if (!config) {
    return { science::StatusCode::kInvalidArgument, "missing config" };
  }

  for (const auto& channel : proto.channels()) {
    config->channels.push_back({
      channel.id(),
      channel.electrode_id(),
      channel.reference_id()
    });
  }
  config->low_cutoff_hz = proto.low_cutoff_hz();
  config->high_cutoff_hz = proto.high_cutoff_hz();
  return {};
}

auto Pixels::to_proto(synapse::PixelConfig* proto) -> science::Status {
  if (proto == nullptr) {
    return { science::StatusCode::kInvalidArgument, "proto ptr must not be null" };
  }
  return {};
}

auto Pixels::from_proto(const synapse::PixelConfig& proto, Pixels* config) -> science::Status {
  if (!config) {
    return { science::StatusCode::kInvalidArgument, "missing config" };
  }
  return {};
}

auto Signal::to_proto(synapse::SignalConfig* proto) -> science::Status {  
  if (proto == nullptr) {
    return { science::StatusCode::kInvalidArgument, "proto ptr must not be null" };
  }
  return {};
}

auto Signal::from_proto(const synapse::SignalConfig& proto, Signal* config) -> science::Status {
  if (!config) {
    return { science::StatusCode::kInvalidArgument, "missing config" };
  }
  return {};
}

}  // namespace synapse
