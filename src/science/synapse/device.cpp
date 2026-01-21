#include "science/synapse/device.h"

namespace synapse {

Device::Device(const std::string& uri)
  : uri_(uri),
    channel_(grpc::CreateChannel(uri, grpc::InsecureChannelCredentials())),
    rpc_(synapse::SynapseDevice::NewStub(channel_)) {}

auto Device::configure(Config* config, std::optional<std::chrono::milliseconds> timeout) -> science::Status {
  auto s = config->set_device(this);
  if (!s.ok()) {
    return { s.code(), "failed to set device: " + s.message() };
  }

  grpc::ClientContext context;
  if (timeout) {
    context.set_deadline(std::chrono::system_clock::now() + *timeout);
  }

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

auto Device::info(synapse::DeviceInfo* info, std::optional<std::chrono::milliseconds> timeout) -> science::Status {
  grpc::ClientContext context;
  if (timeout) {
    context.set_deadline(std::chrono::system_clock::now() + *timeout);
  }

  google::protobuf::Empty req;
  synapse::DeviceInfo res;
  science::Status status;
  bool done = false;
  std::mutex m;
  std::condition_variable cv;

  rpc_->async()->Info(
    &context, &req, &res,
    [this, &res, &status, &done, &m, &cv](grpc::Status gstatus) {
      science::Status s;
      if (!gstatus.ok()) {
        s = { static_cast<science::StatusCode>(gstatus.error_code()), gstatus.error_message() };
      } else {
        s = handle_status_response(res.status());
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

auto Device::start(std::optional<std::chrono::milliseconds> timeout) -> science::Status {
  grpc::ClientContext context;
  if (timeout) {
    context.set_deadline(std::chrono::system_clock::now() + *timeout);
  }

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

auto Device::stop(std::optional<std::chrono::milliseconds> timeout) -> science::Status {
  grpc::ClientContext context;
  if (timeout) {
    context.set_deadline(std::chrono::system_clock::now() + *timeout);
  }

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

auto Device::query(const synapse::QueryRequest& request,
                   synapse::QueryResponse* response,
                   std::optional<std::chrono::milliseconds> timeout) -> science::Status {
  if (response == nullptr) {
    return {science::StatusCode::kInvalidArgument, "response must not be null"};
  }

  grpc::ClientContext context;
  if (timeout) {
    context.set_deadline(std::chrono::system_clock::now() + *timeout);
  }

  synapse::QueryResponse res;
  science::Status status;
  bool done = false;
  std::mutex m;
  std::condition_variable cv;

  rpc_->async()->Query(
    &context, &request, &res,
    [&res, &status, &done, &m, &cv, response](grpc::Status gstatus) {
      science::Status s;
      if (!gstatus.ok()) {
        s = {static_cast<science::StatusCode>(gstatus.error_code()), gstatus.error_message()};
      } else {
        if (res.status().code() != synapse::StatusCode::kOk) {
          s = {science::StatusCode::kInternal,
               "(code: " + std::to_string(res.status().code()) + "): " + res.status().message()};
        }
        response->CopyFrom(res);
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

auto Device::get_logs(const synapse::LogQueryRequest& request,
                       synapse::LogQueryResponse* response,
                       std::optional<std::chrono::milliseconds> timeout) -> science::Status {
  if (response == nullptr) {
    return {science::StatusCode::kInvalidArgument, "response must not be null"};
  }

  grpc::ClientContext context;
  if (timeout) {
    context.set_deadline(std::chrono::system_clock::now() + *timeout);
  }

  synapse::LogQueryResponse res;
  science::Status status;
  bool done = false;
  std::mutex m;
  std::condition_variable cv;

  rpc_->async()->GetLogs(
    &context, &request, &res,
    [&res, &status, &done, &m, &cv, response](grpc::Status gstatus) {
      science::Status s;
      if (!gstatus.ok()) {
        s = {static_cast<science::StatusCode>(gstatus.error_code()), gstatus.error_message()};
      } else {
        response->CopyFrom(res);
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

auto Device::update_settings(const synapse::UpdateDeviceSettingsRequest& request,
                              synapse::UpdateDeviceSettingsResponse* response,
                              std::optional<std::chrono::milliseconds> timeout) -> science::Status {
  if (response == nullptr) {
    return {science::StatusCode::kInvalidArgument, "response must not be null"};
  }

  grpc::ClientContext context;
  if (timeout) {
    context.set_deadline(std::chrono::system_clock::now() + *timeout);
  }

  synapse::UpdateDeviceSettingsResponse res;
  science::Status status;
  bool done = false;
  std::mutex m;
  std::condition_variable cv;

  rpc_->async()->UpdateDeviceSettings(
    &context, &request, &res,
    [&res, &status, &done, &m, &cv, response](grpc::Status gstatus) {
      science::Status s;
      if (!gstatus.ok()) {
        s = {static_cast<science::StatusCode>(gstatus.error_code()), gstatus.error_message()};
      } else {
        if (res.status().code() != synapse::StatusCode::kOk) {
          s = {science::StatusCode::kInternal,
               "(code: " + std::to_string(res.status().code()) + "): " + res.status().message()};
        }
        response->CopyFrom(res);
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

auto Device::list_apps(synapse::ListAppsResponse* response,
                        std::optional<std::chrono::milliseconds> timeout) -> science::Status {
  if (response == nullptr) {
    return {science::StatusCode::kInvalidArgument, "response must not be null"};
  }

  grpc::ClientContext context;
  if (timeout) {
    context.set_deadline(std::chrono::system_clock::now() + *timeout);
  }

  synapse::ListAppsRequest req;
  synapse::ListAppsResponse res;
  science::Status status;
  bool done = false;
  std::mutex m;
  std::condition_variable cv;

  rpc_->async()->ListApps(
    &context, &req, &res,
    [&res, &status, &done, &m, &cv, response](grpc::Status gstatus) {
      science::Status s;
      if (!gstatus.ok()) {
        s = {static_cast<science::StatusCode>(gstatus.error_code()), gstatus.error_message()};
      } else {
        response->CopyFrom(res);
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

  return {};
}

}  // namespace synapse
