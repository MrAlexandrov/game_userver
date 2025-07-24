#include "get.hpp"

#include "models/pack.hpp"
#include "storage/get_all_packs.hpp"

#include <userver/storages/postgres/component.hpp>
#include <samples_postgres_service/sql_queries.hpp>

#include <userver/logging/log.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "utils/string_to_uuid.hpp"

namespace game_userver {

GetPack::GetPack(
    const userver::components::ComponentConfig& config
    , const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context)
    , pg_cluster_(
        component_context.FindComponent<userver::components::Postgres>("postgres-db-1")
            .GetCluster()) 
{
}

std::string GetPack::HandleRequestThrow(
      const userver::server::http::HttpRequest& request
    , userver::server::request::RequestContext&
) const {
    using userver::logging::Level::kDebug;

    const auto& stringUuid = request.GetArg("uuid");

    const auto uuid = NUtils::GetUuidByString(stringUuid);
    if (uuid.is_nil()) {
        return "Incorrect uuid";
    }

    const auto getPackOpt = NStorage::GetPack(pg_cluster_, uuid);
    if (!getPackOpt) {
        return {};
    }
    const auto getPack = getPackOpt.value();

    return userver::formats::json::ToString(
        userver::formats::json::ValueBuilder{getPack}.ExtractValue()
    );
}

} // namespace game_userver
