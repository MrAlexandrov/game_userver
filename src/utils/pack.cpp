#include "pack.hpp"

#include "handlers/cruds.pb.h"
#include "string_to_uuid.hpp"

#include <userver/formats/json/serialize.hpp>

namespace Utils {

auto GetPackDataFromRequest(const userver::server::http::HttpRequest& request)
    -> PackData {
    const auto& body =
        userver::formats::json::FromString(request.RequestBody());
    return PackData{
        .title = body["title"].As<std::string>(),
    };
}

auto GetPackDataFromRequest(handlers::api::CreatePackRequest&& request)
    -> std::expected<PackData, grpc::Status> {
    if (request.title().empty()) {
        return std::unexpected(
            grpc::Status{
                grpc::StatusCode::INVALID_ARGUMENT, "Pack title cannot be empty"
            }
        );
    }
    return PackData{
        .title = request.title(),
    };
}

} // namespace Utils
