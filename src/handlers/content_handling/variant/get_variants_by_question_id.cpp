#include "get_variants_by_question_id.hpp"

#include "storage/variants.hpp"
#include "utils/string_to_uuid.hpp"

#include <userver/storages/postgres/component.hpp>
#include <samples_postgres_service/sql_queries.hpp>

#include <userver/logging/log.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace game_userver {

GetVariantsByQuestionId::GetVariantsByQuestionId(
      const userver::components::ComponentConfig& config
    , const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context)
    , pg_cluster_(
        component_context.FindComponent<userver::components::Postgres>("postgres-db-1")
            .GetCluster())
{
}

std::string GetVariantsByQuestionId::HandleRequestThrow(
      const userver::server::http::HttpRequest& request
    , userver::server::request::RequestContext&
) const {
    using userver::logging::Level::kDebug;

    const auto& stringQuestionId = request.GetArg("question_id");

    const auto GetVariantsByQuestionId = NStorage::GetVariantsByQuestionId(pg_cluster_, NUtils::StringToUuid(stringQuestionId));

    userver::formats::json::ValueBuilder result{userver::formats::common::Type::kArray};

    for (const auto& i : GetVariantsByQuestionId) {
        result.PushBack(i);
    }

    return userver::formats::json::ToPrettyString(result.ExtractValue());
}

} // namespace game_userver
