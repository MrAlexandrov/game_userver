#include "get_questions_by_pack_id.hpp"

#include "storage/questions.hpp"
#include "utils/string_to_uuid.hpp"

#include <samples_postgres_service/sql_queries.hpp>
#include <userver/storages/postgres/component.hpp>

#include <userver/logging/log.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace game_userver {

GetQuestionsByPackId::GetQuestionsByPackId(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context),
      pg_cluster_(
          component_context
              .FindComponent<userver::components::Postgres>("postgres-db-1")
              .GetCluster()
      ) {}

std::string GetQuestionsByPackId::
    HandleRequestThrow(const userver::server::http::HttpRequest& request, userver::server::request::RequestContext&)
        const {
    using userver::logging::Level::kDebug;

    const auto& stringPackId = request.GetArg("pack_id");

    const auto GetQuestionsByPackId = NStorage::GetQuestionsByPackId(
        pg_cluster_, NUtils::StringToUuid(stringPackId)
    );

    userver::formats::json::ValueBuilder result{
        userver::formats::common::Type::kArray
    };

    for (const auto& i : GetQuestionsByPackId) {
        result.PushBack(i);
    }

    return userver::formats::json::ToPrettyString(result.ExtractValue());
}

} // namespace game_userver
