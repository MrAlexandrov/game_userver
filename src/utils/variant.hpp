#pragma once

#include "handlers/cruds.pb.h"
#include "models/variant.hpp"

#include <expected>
#include <grpcpp/support/status.h>
#include <userver/server/http/http_request.hpp>

using VariantData = Models::Variant::VariantData;

namespace Utils {

auto GetVariantDataFromRequest(
    const userver::server::http::HttpRequest& request
) -> VariantData;

auto GetVariantDataFromRequest(handlers::api::CreateVariantRequest&& request)
    -> std::expected<VariantData, grpc::Status>;

} // namespace Utils
