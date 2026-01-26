#include "start_game.hpp"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json.hpp>
#include <userver/logging/log.hpp>

#include "components/game_service/game_service_component.hpp"
#include "utils/string_to_uuid.hpp"

namespace game_userver {

struct StartGame::Impl {
    logic::game::GameService& game_service;

    explicit Impl(const userver::components::ComponentContext& context)
        : game_service(context.FindComponent<components::GameServiceComponent>()
                           .GetGameService()) {}
};

StartGame::StartGame(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context), impl_(component_context) {}

StartGame::~StartGame() = default;

auto StartGame::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& /*context*/
) const -> std::string {
    auto game_session_id_str = request.GetPathArg("game_id");
    auto game_session_id = Utils::StringToUuid(game_session_id_str);

    // JSON body no longer needed for this endpoint

    auto game_session = impl_->game_service.StartGame(game_session_id);

    userver::formats::json::ValueBuilder response;

    if (!game_session.has_value()) {
        request.GetHttpResponse().SetStatus(
            userver::server::http::HttpStatus::kInternalServerError
        );
        response["error"] = "Failed to start game";
        return userver::formats::json::ToString(response.ExtractValue());
    }

    response["id"] = boost::uuids::to_string(game_session->id);
    response["pack_id"] = boost::uuids::to_string(game_session->pack_id);
    response["state"] = game_session->state;
    response["current_question_index"] = game_session->current_question_index;

    return userver::formats::json::ToPrettyString(response.ExtractValue());
}

} // namespace game_userver
