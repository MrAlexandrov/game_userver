#include "get_variant_by_id.hpp"

#include "models/variant.hpp"
#include "storage/variants.hpp"

#include <userver/storages/postgres/component.hpp>
#include <samples_postgres_service/sql_queries.hpp>

#include <userver/logging/log.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "utils/string_to_uuid.hpp"

namespace game_userver {

GetVariantById::GetVariantById(
      const userver::components::ComponentConfig& config
    , const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context)
    , pg_cluster_(
        component_context.FindComponent<userver::components::Postgres>("postgres-db-1")
            .GetCluster())
{
}

std::string GetVariantById::HandleRequestThrow(
      const userver::server::http::HttpRequest& request
    , userver::server::request::RequestContext&
) const {
    using userver::logging::Level::kDebug;

    const auto& stringId = request.GetArg("id");

    const auto id = NUtils::StringToUuid(stringId);
    if (id.is_nil()) {
        return "id";
    }

    const auto GetVariantByIdOpt = NStorage::GetVariantById(pg_cluster_, id);
    if (!GetVariantByIdOpt) {
        return {};
    }
    const auto GetVariantById = GetVariantByIdOpt.value();

    return userver::formats::json::ToPrettyString(
        userver::formats::json::ValueBuilder{GetVariantById}.ExtractValue()
    );
}

} // namespace game_userver
