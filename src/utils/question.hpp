#pragma once

#include "handlers/cruds.pb.h"
#include "models/question.hpp"

#include <expected>
#include <grpcpp/support/status.h>
#include <userver/server/http/http_request.hpp>

namespace Utils {

auto GetQuestionFromRequest(const userver::server::http::HttpRequest& request)
    -> Models::Question;

auto GetQuestionFromRequest(handlers::api::CreateQuestionRequest&& request)
    -> std::expected<Models::Question, grpc::Status>;

} // namespace Utils
