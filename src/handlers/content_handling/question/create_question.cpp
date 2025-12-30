#include "create_question.hpp"

#include "storage/questions.hpp"
#include "utils/string_to_uuid.hpp"

#include <samples_postgres_service/sql_queries.hpp>
#include <userver/storages/postgres/component.hpp>

#include <userver/logging/log.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace game_userver {

CreateQuestion::CreateQuestion(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context),
      pg_cluster_(
          component_context
              .FindComponent<userver::components::Postgres>("postgres-db-1")
              .GetCluster()
      ) {}

std::string CreateQuestion::
    HandleRequestThrow(const userver::server::http::HttpRequest& request, userver::server::request::RequestContext&)
        const {
    using userver::logging::Level::kDebug;

    const auto& pack_id = request.GetArg("pack_id");
    const auto& text = request.GetArg("text");
    const auto& image_url = request.GetArg("image_url");

    const auto createdQuestionOpt = NStorage::CreateQuestion(
        pg_cluster_, NUtils::StringToUuid(pack_id), text, image_url
    );
    const auto createdQuestion = createdQuestionOpt.value();

    return userver::formats::json::ToPrettyString(
        userver::formats::json::ValueBuilder{createdQuestion}.ExtractValue()
    );
}

} // namespace game_userver
