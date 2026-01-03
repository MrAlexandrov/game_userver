#include "add_player.hpp"

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

struct AddPlayer::Impl {
    userver::storages::postgres::ClusterPtr pg_cluster;

    explicit Impl(const userver::components::ComponentContext& context)
        : pg_cluster(context
                         .FindComponent<userver::components::Postgres>(
                             Constants::kDatabaseName
                         )
                         .GetCluster()) {}
};

AddPlayer::AddPlayer(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context), impl_(component_context) {}

AddPlayer::~AddPlayer() = default;

auto AddPlayer::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& /*context*/
) const -> std::string {
    auto game_session_id_str = request.GetPathArg("game_id");
    auto game_session_id = Utils::StringToUuid(game_session_id_str);

    const auto& request_body = request.RequestBody();
    auto json = userver::formats::json::FromString(request_body);
    auto player_name = json["player_name"].As<std::string>();

    logic::game::GameService game_service(impl_->pg_cluster);
    auto player = game_service.AddPlayer(game_session_id, player_name);

    userver::formats::json::ValueBuilder response;

    if (!player.has_value()) {
        request.GetHttpResponse().SetStatus(
            userver::server::http::HttpStatus::kInternalServerError
        );
        response["error"] = "Failed to add player";
        return userver::formats::json::ToPrettyString(response.ExtractValue());
    }

    response["id"] = boost::uuids::to_string(player->id);
    response["game_session_id"] =
        boost::uuids::to_string(player->game_session_id);
    response["name"] = player->name;
    response["score"] = player->score;

    return userver::formats::json::ToPrettyString(response.ExtractValue());
}

} // namespace game_userver
