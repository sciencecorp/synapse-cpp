#pragma once

#include <memory>
#include <string>
#include "science/synapse/api/nodes/disk_writer.pb.h"
#include "science/synapse/node.h"

namespace synapse {

class DiskWriter : public Node {
 public:
  explicit DiskWriter(const std::string& filename);

  [[nodiscard]] static auto from_proto(
    const synapse::NodeConfig& proto,
    std::shared_ptr<Node>* node
  ) -> science::Status;

 protected:
  auto p_to_proto(synapse::NodeConfig* proto) -> science::Status override;

 private:
  std::string filename_;
};

}  // namespace synapse
