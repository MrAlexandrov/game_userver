#include "get_pack_by_id.hpp"

#include "storage/packs.hpp"
#include "utils/string_to_uuid.hpp"

#include <samples_postgres_service/sql_queries.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

namespace game_userver {

struct GetPack::Impl {
    userver::storages::postgres::ClusterPtr pg_cluster;

    explicit Impl(const userver::components::ComponentContext& context)
        : pg_cluster(
              context
                  .FindComponent<userver::components::Postgres>("postgres-db-1")
                  .GetCluster()
          ) {}
};

GetPack::GetPack(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context), impl_(component_context) {}

GetPack::~GetPack() = default;

std::string GetPack::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    const auto& stringUuid = request.GetArg("uuid");

    const auto uuid = NUtils::StringToUuid(stringUuid);
    if (uuid.is_nil()) {
        return "Incorrect uuid";
    }

    const auto packOpt = NStorage::GetPackById(impl_->pg_cluster, uuid);
    if (!packOpt) {
        return {};
    }

    return userver::formats::json::ToPrettyString(
        userver::formats::json::ValueBuilder{packOpt.value()}.ExtractValue()
    );
}

} // namespace game_userver
