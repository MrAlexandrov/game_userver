#include "submit_answer.hpp"

#include <boost/uuid/uuid.hpp> // NOLINT
#include <boost/uuid/uuid_io.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json.hpp>
#include <userver/formats/parse/common_containers.hpp>
#include <userver/logging/log.hpp>

#include "components/game_service/game_service_component.hpp"
#include "utils/string_to_uuid.hpp"

namespace game_userver {

struct SubmitAnswer::Impl {
    logic::game::GameService& game_service;

    explicit Impl(const userver::components::ComponentContext& context)
        : game_service(context.FindComponent<components::GameServiceComponent>()
                           .GetGameService()) {}
};

SubmitAnswer::SubmitAnswer(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context), impl_(component_context) {}

SubmitAnswer::~SubmitAnswer() = default;

auto SubmitAnswer::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& /*context*/
) const -> std::string {
    // auto game_session_id_str = request.GetPathArg("game_id");

    const auto& request_body = request.RequestBody();
    auto json = userver::formats::json::FromString(request_body);

    auto player_id_str = json["player_id"].As<std::string>();
    auto player_id = Utils::StringToUuid(player_id_str);

    logic::validators::PlayerAnswerInput answer_input;

    auto variant_id_str = json["variant_id"].As<std::optional<std::string>>();
    if (variant_id_str.has_value() && !variant_id_str->empty()) {
        answer_input.variant_id = Utils::StringToUuid(*variant_id_str);
    }

    auto text_answer = json["text_answer"].As<std::optional<std::string>>();
    if (text_answer.has_value()) {
        answer_input.text_answer = std::move(*text_answer);
    }

    auto result = impl_->game_service.SubmitAnswer(player_id, answer_input);

    userver::formats::json::ValueBuilder response;

    switch (result) {
    case logic::game::GameResult::kCorrect:
        response["result"] = "correct";
        response["game_finished"] = false;
        break;
    case logic::game::GameResult::kIncorrect:
        response["result"] = "incorrect";
        response["game_finished"] = false;
        break;
    case logic::game::GameResult::kGameFinished:
        response["result"] = "game_finished";
        response["game_finished"] = true;
        break;
    case logic::game::GameResult::kError:
        request.GetHttpResponse().SetStatus(
            userver::server::http::HttpStatus::kInternalServerError
        );
        response["error"] = "Failed to submit answer";
        break;
    }

    return userver::formats::json::ToPrettyString(response.ExtractValue());
}

} // namespace game_userver
