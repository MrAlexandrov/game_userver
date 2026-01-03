#include "get_game_state.hpp"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json.hpp>
#include <userver/logging/log.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

#include "logic/game/game.hpp"
#include "utils/constants.hpp"
#include "utils/string_to_uuid.hpp"

namespace game_userver {

struct GetGameState::Impl {
    userver::storages::postgres::ClusterPtr pg_cluster;

    explicit Impl(const userver::components::ComponentContext& context)
        : pg_cluster(context
                         .FindComponent<userver::components::Postgres>(
                             Constants::kDatabaseName
                         )
                         .GetCluster()) {}
};

GetGameState::GetGameState(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context), impl_(component_context) {}

GetGameState::~GetGameState() = default;

auto GetGameState::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& /*context*/
) const -> std::string {
    auto game_session_id_str = request.GetPathArg("game_id");
    auto game_session_id = Utils::StringToUuid(game_session_id_str);

    logic::game::GameService game_service(impl_->pg_cluster);
    auto game_question = game_service.GetCurrentQuestion(game_session_id);

    userver::formats::json::ValueBuilder response;

    if (!game_question.has_value()) {
        request.GetHttpResponse().SetStatus(
            userver::server::http::HttpStatus::kNotFound
        );
        response["error"] = "Question not found or game finished";
        return userver::formats::json::ToString(response.ExtractValue());
    }

    response["question"]["id"] =
        boost::uuids::to_string(game_question->question.id);
    response["question"]["text"] = game_question->question.data.text;
    if (!game_question->question.data.image_url.empty()) {
        response["question"]["image_url"] =
            game_question->question.data.image_url;
    }

    auto variants_array = response["variants"];
    for (size_t i = 0; i < game_question->variants.size(); ++i) {
        const auto& variant = game_question->variants[i];
        variants_array[i]["id"] = boost::uuids::to_string(variant.id);
        variants_array[i]["text"] = variant.data.text;
        // Don't send is_correct to client!
    }

    return userver::formats::json::ToPrettyString(response.ExtractValue());
}

} // namespace game_userver
