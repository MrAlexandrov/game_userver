#pragma once

#include "handlers/cruds.pb.h"
#include "models/pack.hpp"

#include <expected>
#include <grpcpp/support/status.h>
#include <userver/server/http/http_request.hpp>

namespace Utils {

// HTTP
auto GetPackFromRequest(const userver::server::http::HttpRequest& request)
    -> Models::Pack;

Models::Pack::PackId
GetPackIdFromRequest(const userver::server::http::HttpRequest& request);

// ---

// GRPC
auto GetPackFromRequest(handlers::api::CreatePackRequest&& request)
    -> std::expected<Models::Pack, grpc::Status>;

} // namespace Utils
