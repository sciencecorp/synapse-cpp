#include "science/synapse/device.h"

namespace synapse {

Device::Device(const std::string& uri)
  : uri_(uri),
    channel_(grpc::CreateChannel(uri, grpc::InsecureChannelCredentials())),
    rpc_(synapse::SynapseDevice::NewStub(channel_)) {}

bool Device::configure(Config* config) {
  config->set_device(this);

  grpc::ClientContext context;

  synapse::DeviceConfiguration req = config->to_proto();
  synapse::Status res;
  bool ok = false;
  bool done = false;
  std::mutex m;
  std::condition_variable cv;

  rpc_->async()->Configure(
    &context, &req, &res,
    [this, &res, &ok, &done, &m, &cv](grpc::Status status) mutable {
      bool _ok = status.ok();
      if (!ok) {
        _ok = handle_status_response(res);
      }

      std::lock_guard<std::mutex> lock(m);
      done = true;
      ok = _ok;
      cv.notify_one();
    });

  std::unique_lock<std::mutex> lock(m);
  cv.wait(lock, [&done] { return done; });
  return ok;
}

std::optional<synapse::DeviceInfo> Device::info() {
  grpc::ClientContext context;

  google::protobuf::Empty req;
  synapse::DeviceInfo res;
  bool ok = false;
  bool done = false;
  std::mutex m;
  std::condition_variable cv;

  rpc_->async()->Info(
    &context, &req, &res,
    [&ok, &done, &m, &cv](grpc::Status status) {
      bool _ok = status.ok();
      if (!_ok) {
        _ok = false;
      }

      std::lock_guard<std::mutex> lock(m);
      done = true;
      ok = _ok;
      cv.notify_one();
    });

  std::unique_lock<std::mutex> lock(m);
  cv.wait(lock, [&done] { return done; });
  if (!ok) {
    return std::nullopt;
  }
  return res;
}

bool Device::start() {
  grpc::ClientContext context;

  google::protobuf::Empty req;
  synapse::Status res;
  bool ok = false;
  bool done = false;
  std::mutex m;
  std::condition_variable cv;

  rpc_->async()->Start(
    &context, &req, &res,
    [this, &res, &ok, &done, &m, &cv](grpc::Status status) {
      bool _ok = status.ok();
      if (!ok) {
        _ok = handle_status_response(res);
      }

      std::lock_guard<std::mutex> lock(m);
      done = true;
      ok = _ok;
      cv.notify_one();
    });

  std::unique_lock<std::mutex> lock(m);
  cv.wait(lock, [&done] { return done; });
  return ok;
}

bool Device::stop() {
  grpc::ClientContext context;

  google::protobuf::Empty req;
  synapse::Status res;
  bool ok = false;
  bool done = false;
  std::mutex m;
  std::condition_variable cv;

  rpc_->async()->Stop(
    &context, &req, &res,
    [this, &res, &ok, &done, &m, &cv](grpc::Status status) {
      bool _ok = status.ok();
      if (!ok) {
        _ok = handle_status_response(res);
      }

      std::lock_guard<std::mutex> lock(m);
      done = true;
      ok = _ok;
      cv.notify_one();
    });

  std::unique_lock<std::mutex> lock(m);
  cv.wait(lock, [&done] { return done; });
  return ok;
}

bool Device::handle_status_response(const synapse::Status& status) {
  if (status.code() != synapse::StatusCode::kOk) {
    return false;
  }

  sockets_.clear();
  for (const auto& socket : status.sockets()) {
    synapse::NodeSocket s;
    s.CopyFrom(socket);
    sockets_.push_back(s);
  }

  return true;
}

}  // namespace synapse
