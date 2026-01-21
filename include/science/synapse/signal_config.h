#pragma once

#include <variant>
#include <vector>
#include "science/synapse/api/nodes/signal_config.pb.h"
#include "science/synapse/channel.h"
#include "science/synapse/status.h"

namespace synapse {

struct Electrodes {
  std::vector<Ch> channels;
  float low_cutoff_hz;
  float high_cutoff_hz;

  [[nodiscard]] auto to_proto(synapse::ElectrodeConfig* proto) -> science::Status;
  [[nodiscard]] auto static from_proto(const synapse::ElectrodeConfig& proto, Electrodes* config) -> science::Status;
};

struct Pixels {
  std::vector<uint32_t> pixel_mask;

  [[nodiscard]] auto to_proto(synapse::PixelConfig* proto) -> science::Status;
  [[nodiscard]] auto static from_proto(const synapse::PixelConfig& proto, Pixels* config) -> science::Status;
};

struct Signal {
  std::variant<Electrodes, Pixels> signal;

  [[nodiscard]] auto to_proto(synapse::SignalConfig* proto) -> science::Status;
  [[nodiscard]] auto static from_proto(const synapse::SignalConfig& proto, Signal* config) -> science::Status;
};

}  // namespace synapse
