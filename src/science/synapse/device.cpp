#include "science/synapse/device.h"

namespace synapse {

Device::Device(const std::string& uri)
  : uri_(uri),
    channel_(grpc::CreateChannel(uri, grpc::InsecureChannelCredentials())),
    rpc_(synapse::SynapseDevice::NewStub(channel_)) {}

auto Device::configure(Config* config) -> science::Status {
  auto s = config->set_device(this);
  if (!s.ok()) {
    return { s.code(), "failed to set device: " + s.message() };
  }

  grpc::ClientContext context;

  synapse::DeviceConfiguration req = config->to_proto();
  synapse::Status res;
  science::Status status;
  bool done = false;
  std::mutex m;
  std::condition_variable cv;

  rpc_->async()->Configure(
    &context, &req, &res,
    [this, &res, &status, &done, &m, &cv](grpc::Status gstatus) mutable {
      science::Status s;
      if (!gstatus.ok()) {
        s = { static_cast<science::StatusCode>(gstatus.error_code()), gstatus.error_message() };
      } else {
        s = handle_status_response(res);
      }

      std::lock_guard<std::mutex> lock(m);
      done = true;
      status = s;
      cv.notify_one();
    });

  std::unique_lock<std::mutex> lock(m);
  cv.wait(lock, [&done] { return done; });
  return status;
}

auto Device::info(synapse::DeviceInfo* info) -> science::Status {
  grpc::ClientContext context;

  google::protobuf::Empty req;
  synapse::DeviceInfo res;
  science::Status status;
  bool done = false;
  std::mutex m;
  std::condition_variable cv;

  rpc_->async()->Info(
    &context, &req, &res,
    [&status, &done, &m, &cv](grpc::Status gstatus) {
      science::Status s;
      if (!gstatus.ok()) {
        s = { static_cast<science::StatusCode>(gstatus.error_code()), gstatus.error_message() };
      }

      std::lock_guard<std::mutex> lock(m);
      done = true;
      status = s;
      cv.notify_one();
    });

  std::unique_lock<std::mutex> lock(m);
  cv.wait(lock, [&done] { return done; });
  if (status.ok()) {
    info->CopyFrom(res);
  }
  return status;
}

auto Device::start() -> science::Status {
  grpc::ClientContext context;

  google::protobuf::Empty req;
  synapse::Status res;
  science::Status status;
  bool done = false;
  std::mutex m;
  std::condition_variable cv;

  rpc_->async()->Start(
    &context, &req, &res,
    [this, &res, &status, &done, &m, &cv](grpc::Status gstatus) {
      science::Status s;
      if (!gstatus.ok()) {
        s = { static_cast<science::StatusCode>(gstatus.error_code()), gstatus.error_message() };
      } else {
        s = handle_status_response(res);
      }

      std::lock_guard<std::mutex> lock(m);
      done = true;
      status = s;
      cv.notify_one();
    });

  std::unique_lock<std::mutex> lock(m);
  cv.wait(lock, [&done] { return done; });
  return status;
}

auto Device::stop() -> science::Status {
  grpc::ClientContext context;

  google::protobuf::Empty req;
  synapse::Status res;
  science::Status status;
  bool done = false;
  std::mutex m;
  std::condition_variable cv;

  rpc_->async()->Stop(
    &context, &req, &res,
    [this, &res, &status, &done, &m, &cv](grpc::Status gstatus) {
      science::Status s;
      if (!gstatus.ok()) {
        s = { static_cast<science::StatusCode>(gstatus.error_code()), gstatus.error_message() };
      } else {
        s = handle_status_response(res);
      }

      std::lock_guard<std::mutex> lock(m);
      done = true;
      status = s;
      cv.notify_one();
    });

  std::unique_lock<std::mutex> lock(m);
  cv.wait(lock, [&done] { return done; });
  return status;
}

auto Device::sockets() const -> const std::vector<synapse::NodeSocket>& {
  return sockets_;
}

auto Device::uri() const -> const std::string& {
  return uri_;
}

auto Device::handle_status_response(const synapse::Status& status) -> science::Status {
  if (status.code() != synapse::StatusCode::kOk) {
    return {
      science::StatusCode::kInternal,
      "(code: " + std::to_string(status.code()) + "): " + status.message()
    };
  }

  sockets_.clear();
  for (const auto& socket : status.sockets()) {
    synapse::NodeSocket s;
    s.CopyFrom(socket);
    sockets_.push_back(s);
  }

  return {};
}

}  // namespace synapse
