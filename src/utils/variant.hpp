#pragma once

#include "handlers/cruds.pb.h"
#include "models/variant.hpp"

#include <expected>
#include <grpcpp/support/status.h>
#include <userver/server/http/http_request.hpp>

namespace Utils {

auto GetVariantFromRequest(const userver::server::http::HttpRequest& request)
    -> Models::Variant;

auto GetVariantFromRequest(handlers::api::CreateVariantRequest&& request)
    -> std::expected<Models::Variant, grpc::Status>;

} // namespace Utils
