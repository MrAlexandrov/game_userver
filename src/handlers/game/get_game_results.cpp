#include "get_game_results.hpp"

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

struct GetGameResults::Impl {
    userver::storages::postgres::ClusterPtr pg_cluster;

    explicit Impl(const userver::components::ComponentContext& context)
        : pg_cluster(context
                         .FindComponent<userver::components::Postgres>(
                             Constants::kDatabaseName
                         )
                         .GetCluster()) {}
};

GetGameResults::GetGameResults(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context), impl_(component_context) {}

GetGameResults::~GetGameResults() = default;

auto GetGameResults::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& /*context*/
) const -> std::string {
    auto game_session_id_str = request.GetPathArg("game_id");
    auto game_session_id = Utils::StringToUuid(game_session_id_str);

    logic::game::GameService game_service(impl_->pg_cluster);

    userver::formats::json::ValueBuilder response;

    auto game_session = game_service.GetGameSession(game_session_id);
    if (!game_session.has_value()) {
        request.GetHttpResponse().SetStatus(
            userver::server::http::HttpStatus::kNotFound
        );
        response["error"] = "Game session not found";
        return userver::formats::json::ToString(response.ExtractValue());
    }

    auto players = game_service.GetPlayers(game_session_id);

    response["game_session"]["id"] = boost::uuids::to_string(game_session->id);
    response["game_session"]["state"] = game_session->state;
    response["game_session"]["current_question_index"] =
        game_session->current_question_index;

    auto players_array = response["players"];
    for (size_t i = 0; i < players.size(); ++i) {
        const auto& player = players[i];
        players_array[i]["id"] = boost::uuids::to_string(player.id);
        players_array[i]["name"] = player.name;
        players_array[i]["score"] = player.score;
    }

    return userver::formats::json::ToPrettyString(response.ExtractValue());
}

} // namespace game_userver
