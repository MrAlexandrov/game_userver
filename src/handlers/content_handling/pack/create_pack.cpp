#include "create_pack.hpp"

#include "models/pack.hpp"
#include "storage/pasks.hpp"

#include <userver/storages/postgres/component.hpp>
#include <samples_postgres_service/sql_queries.hpp>

#include <userver/logging/log.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace game_userver {

CreatePack::CreatePack(
    const userver::components::ComponentConfig& config
    , const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context)
    , pg_cluster_(
        component_context.FindComponent<userver::components::Postgres>("postgres-db-1")
            .GetCluster()) 
{
}

std::string CreatePack::HandleRequestThrow(
      const userver::server::http::HttpRequest& request
    , userver::server::request::RequestContext&
) const {
    using userver::logging::Level::kDebug;

    const auto& title = request.GetArg("title");
    LOG(kDebug) << "title: " << title;

    const auto createdPackOpt = NStorage::CreatePack(pg_cluster_, title);
    const auto createdPack = createdPackOpt.value();
    if (createdPackOpt) {
        LOG(kDebug)
            << "inserted pack:\n" 
            << boost::uuids::to_string(createdPack.id)
            << " " << createdPack.title;
    }

    return userver::formats::json::ToPrettyString(
        userver::formats::json::ValueBuilder{createdPack}.ExtractValue()
    );
}

} // namespace game_userver
