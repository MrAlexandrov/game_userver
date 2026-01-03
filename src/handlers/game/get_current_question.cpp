#include "get_current_question.hpp"

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

struct GetCurrentQuestion::Impl {
    userver::storages::postgres::ClusterPtr pg_cluster;

    explicit Impl(const userver::components::ComponentContext& context)
        : pg_cluster(context
                         .FindComponent<userver::components::Postgres>(
                             Constants::kDatabaseName
                         )
                         .GetCluster()) {}
};

GetCurrentQuestion::GetCurrentQuestion(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context), impl_(component_context) {}

GetCurrentQuestion::~GetCurrentQuestion() = default;

auto GetCurrentQuestion::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& /*context*/
) const -> std::string {
    auto game_session_id_str = request.GetArg("game_session_id");
    auto game_session_id = Utils::StringToUuid(game_session_id_str);

    logic::game::GameService game_service(impl_->pg_cluster);
    auto game_question = game_service.GetCurrentQuestion(game_session_id);

    if (!game_question) {
        request.GetHttpResponse().SetStatus(
            userver::server::http::HttpStatus::kNotFound
        );
        return userver::formats::json::ToString(
            userver::formats::json::ValueBuilder{
                {"error", "Question not found or game finished"}
        }
                .ExtractValue()
        );
    }

    userver::formats::json::ValueBuilder response;
    response["question"]["id"] =
        boost::uuids::to_string(game_question->question.id);
    response["question"]["text"] = game_question->question.text;
    if (game_question->question.image_url) {
        response["question"]["image_url"] =
            game_question->question.image_url.value();
    }

    auto variants_array = response["variants"];
    for (size_t i = 0; i < game_question->variants.size(); ++i) {
        const auto& variant = game_question->variants[i];
        variants_array[i]["id"] = boost::uuids::to_string(variant.id);
        variants_array[i]["text"] = variant.text;
        // Don't send is_correct to client!
    }

    return userver::formats::json::ToPrettyString(response.ExtractValue());
}

} // namespace game_userver
