#include "get_all_packs.hpp"

#include "storage/packs.hpp"
#include "utils/string_to_uuid.hpp"

#include <samples_postgres_service/sql_queries.hpp>
#include <userver/storages/postgres/component.hpp>

#include <userver/logging/log.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace game_userver {

GetAllPacks::GetAllPacks(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context),
      pg_cluster_(
          component_context
              .FindComponent<userver::components::Postgres>("postgres-db-1")
              .GetCluster()
      ) {}

std::string GetAllPacks::
    HandleRequestThrow(const userver::server::http::HttpRequest& request, userver::server::request::RequestContext&)
        const {
    using userver::logging::Level::kDebug;

    const auto getAllPacks = NStorage::GetAllPacks(pg_cluster_);

    userver::formats::json::ValueBuilder result{
        userver::formats::common::Type::kArray
    };

    for (const auto& i : getAllPacks) {
        result.PushBack(i);
    }

    return userver::formats::json::ToPrettyString(result.ExtractValue());
}

} // namespace game_userver
