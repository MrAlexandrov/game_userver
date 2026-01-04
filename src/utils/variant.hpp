#pragma once

#include "handlers/cruds.pb.h"
#include "models/variant.hpp"

#include <expected>
#include <grpcpp/support/status.h>
#include <userver/server/http/http_request.hpp>

namespace Utils {

struct VariantData {
    boost::uuids::uuid question_id;
    std::string text;
    bool is_correct;
};

auto GetVariantDataFromRequest(
    const userver::server::http::HttpRequest& request
) -> VariantData;

auto GetVariantDataFromRequest(handlers::api::CreateVariantRequest&& request)
    -> std::expected<VariantData, grpc::Status>;

} // namespace Utils
