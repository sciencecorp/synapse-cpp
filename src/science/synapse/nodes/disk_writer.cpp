#include "science/synapse/nodes/disk_writer.h"

namespace synapse {

DiskWriter::DiskWriter(const std::string& filename)
    : Node(NodeType::kDiskWriter),
      filename_(filename) {}

auto DiskWriter::from_proto(const synapse::NodeConfig& proto,
                            std::shared_ptr<Node>* node) -> science::Status {
  if (!proto.has_disk_writer()) {
    return {science::StatusCode::kInvalidArgument, "missing disk_writer config"};
  }

  const auto& config = proto.disk_writer();

  *node = std::make_shared<DiskWriter>(config.filename());

  return {};
}

auto DiskWriter::p_to_proto(synapse::NodeConfig* proto) -> science::Status {
  if (proto == nullptr) {
    return {science::StatusCode::kInvalidArgument, "proto ptr must not be null"};
  }

  synapse::DiskWriterConfig* config = proto->mutable_disk_writer();
  config->set_filename(filename_);

  return {};
}

}  // namespace synapse
