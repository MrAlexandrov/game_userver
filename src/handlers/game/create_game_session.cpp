#include "create_game_session.hpp"

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

struct CreateGameSession::Impl {
    userver::storages::postgres::ClusterPtr pg_cluster;

    explicit Impl(const userver::components::ComponentContext& context)
        : pg_cluster(context
                         .FindComponent<userver::components::Postgres>(
                             Constants::kDatabaseName
                         )
                         .GetCluster()) {}
};

CreateGameSession::CreateGameSession(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context), impl_(component_context) {}

CreateGameSession::~CreateGameSession() = default;

auto CreateGameSession::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& /*context*/
) const -> std::string {
    const auto& request_body = request.RequestBody();
    auto json = userver::formats::json::FromString(request_body);

    auto pack_id_str = json["pack_id"].As<std::string>();
    auto pack_id = Utils::StringToUuid(pack_id_str);

    logic::game::GameService game_service(impl_->pg_cluster);
    auto game_session = game_service.CreateGameSession(pack_id);

    userver::formats::json::ValueBuilder response;

    if (!game_session.has_value()) {
        request.GetHttpResponse().SetStatus(
            userver::server::http::HttpStatus::kInternalServerError
        );
        response["error"] = "Failed to create game session";
        return userver::formats::json::ToString(response.ExtractValue());
    }

    response["id"] = boost::uuids::to_string(game_session->id);
    response["pack_id"] = boost::uuids::to_string(game_session->pack_id);
    response["state"] = game_session->state;
    response["current_question_index"] = game_session->current_question_index;

    return userver::formats::json::ToPrettyString(response.ExtractValue());
}

} // namespace game_userver
