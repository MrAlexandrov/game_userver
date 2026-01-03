#pragma once

#include "handlers/cruds.pb.h"
#include "models/question.hpp"

#include <expected>
#include <grpcpp/support/status.h>
#include <userver/server/http/http_request.hpp>

using QuestionData = Models::Question::QuestionData;

namespace Utils {

auto GetQuestionDataFromRequest(
    const userver::server::http::HttpRequest& request
) -> QuestionData;

auto GetQuestionDataFromRequest(handlers::api::CreateQuestionRequest&& request)
    -> std::expected<QuestionData, grpc::Status>;

} // namespace Utils
