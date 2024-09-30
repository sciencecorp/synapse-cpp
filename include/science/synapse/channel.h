#pragma once

#include "science/synapse/api/channel.pb.h"

namespace synapse {

struct Ch {
  uint64_t id;
  uint64_t electrode_id;
  uint64_t reference_id;

  auto to_proto(synapse::Channel* proto) const -> void {
    proto->set_id(id);
    proto->set_electrode_id(electrode_id);
    proto->set_reference_id(reference_id);
  }
};

}  // namespace synapse
