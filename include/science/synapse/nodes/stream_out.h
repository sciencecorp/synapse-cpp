#pragma once

#include <netinet/in.h>
#include <memory>
#include <string>
#include <vector>

#include "science/libndtp/types.h"
#include "science/scipp/status.h"
#include "science/synapse/api/nodes/stream_out.pb.h"
#include "science/synapse/node.h"

namespace synapse {

class StreamOut : public Node {
 public:
  static constexpr uint16_t DEFAULT_STREAM_OUT_PORT = 50038;
  StreamOut(const std::string& destination_address = "",
           uint16_t destination_port = DEFAULT_STREAM_OUT_PORT,
           const std::string& label = "");
  ~StreamOut();

  auto init() -> science::Status;
  auto read(science::libndtp::SynapseData* out, science::libndtp::NDTPHeader* header = nullptr, size_t* bytes_read = nullptr) -> science::Status;

  [[nodiscard]] static auto from_proto(
    const synapse::NodeConfig& proto,
    std::shared_ptr<Node>* node
  ) -> science::Status;

 protected:
  auto p_to_proto(synapse::NodeConfig* proto) -> science::Status override;

 private:
  std::string destination_address_;
  uint16_t destination_port_;
  std::string label_;
  int socket_ = 0;
  std::optional<sockaddr_in> addr_;

  static constexpr uint32_t SOCKET_BUFSIZE_BYTES = 5 * 1024 * 1024;  // 5MB
};

}  // namespace synapse
