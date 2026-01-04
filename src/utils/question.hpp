#pragma once

#include "handlers/cruds.pb.h"
#include "models/question.hpp"

#include <expected>
#include <grpcpp/support/status.h>
#include <userver/server/http/http_request.hpp>

namespace Utils {

struct QuestionData {
    boost::uuids::uuid pack_id;
    std::string text;
    std::string image_url;
};

auto GetQuestionDataFromRequest(
    const userver::server::http::HttpRequest& request
) -> QuestionData;

auto GetQuestionDataFromRequest(handlers::api::CreateQuestionRequest&& request)
    -> std::expected<QuestionData, grpc::Status>;

} // namespace Utils
