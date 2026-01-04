#pragma once

#include "handlers/cruds.pb.h"
#include "models/pack.hpp"

#include <expected>
#include <grpcpp/support/status.h>
#include <userver/server/http/http_request.hpp>

namespace Utils {

struct PackData {
    std::string title;
};

auto GetPackDataFromRequest(const userver::server::http::HttpRequest& request)
    -> PackData;

auto GetPackDataFromRequest(handlers::api::CreatePackRequest&& request)
    -> std::expected<PackData, grpc::Status>;

} // namespace Utils
