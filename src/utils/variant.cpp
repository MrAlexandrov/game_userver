#include "variant.hpp"

#include "handlers/cruds.pb.h"
#include "string_to_uuid.hpp"

#include <userver/formats/json/serialize.hpp>

namespace Utils {

auto GetVariantDataFromRequest(
    const userver::server::http::HttpRequest& request
) -> VariantData {
    const auto& body =
        userver::formats::json::FromString(request.RequestBody());
    return VariantData{
        .question_id =
            Utils::StringToUuid(""), // Will be set in handler from path
        .text = body["text"].As<std::string>(),
        .is_correct = Utils::StringToBool(body["is_correct"].As<std::string>()),
    };
}

auto GetVariantDataFromRequest(handlers::api::CreateVariantRequest&& request)
    -> std::expected<VariantData, grpc::Status> {
    if (request.text().empty()) {
        return std::unexpected(
            grpc::Status{
                grpc::StatusCode::INVALID_ARGUMENT,
                "Variant text cannot be empty"
            }
        );
    }
    const auto question_id = Utils::StringToUuid(request.question_id());
    if (question_id.is_nil()) {
        return std::unexpected(
            grpc::Status{
                grpc::StatusCode::INVALID_ARGUMENT,
                "Invalid UUID format: " + request.question_id()
            }
        );
    }
    return VariantData{
        .question_id = Utils::StringToUuid(request.question_id()),
        .text = request.text(),
        .is_correct = request.is_correct(),
    };
}

} // namespace Utils
