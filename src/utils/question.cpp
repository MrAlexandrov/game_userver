#include "question.hpp"

#include "handlers/cruds.pb.h"
#include "string_to_uuid.hpp"

#include <userver/formats/json/serialize.hpp>

namespace Utils {

auto GetQuestionDataFromRequest(
    const userver::server::http::HttpRequest& request
) -> QuestionData {
    const auto& body =
        userver::formats::json::FromString(request.RequestBody());
    return QuestionData{
        .pack_id = Utils::StringToUuid(body["pack_id"].As<std::string>()),
        .text = body["text"].As<std::string>(),
        .image_url = body["image_url"].As<std::string>(),
    };
}

auto GetQuestionDataFromRequest(handlers::api::CreateQuestionRequest&& request)
    -> std::expected<QuestionData, grpc::Status> {
    if (request.text().empty()) {
        return std::unexpected(
            grpc::Status{
                grpc::StatusCode::INVALID_ARGUMENT,
                "Question text cannot be empty"
            }
        );
    }
    const auto pack_id = Utils::StringToUuid(request.pack_id());
    if (pack_id.is_nil()) {
        return std::unexpected(
            grpc::Status{
                grpc::StatusCode::INVALID_ARGUMENT,
                "Invalid UUID format: " + request.pack_id()
            }
        );
    }
    return QuestionData{
        .pack_id = Utils::StringToUuid(request.pack_id()),
        .text = request.text(),
        .image_url = request.image_url(),
    };
}

} // namespace Utils
