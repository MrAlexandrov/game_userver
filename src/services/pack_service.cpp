#include "pack_service.hpp"

#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

#include "storage/packs.hpp"
#include "utils/constants.hpp"

namespace game_userver::services {

PackService::PackService(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : LoggableComponentBase(config, context),
      pg_cluster_(context
                      .FindComponent<userver::components::Postgres>(
                          Constants::kDatabaseName
                      )
                      .GetCluster()) {}

PackService::~PackService() = default;

std::optional<Models::Pack> PackService::CreatePack(const std::string& title) {
    return NStorage::CreatePack(pg_cluster_, title);
}

std::optional<Models::Pack>
PackService::GetPackById(const boost::uuids::uuid& pack_id) {
    return NStorage::GetPackById(pg_cluster_, pack_id);
}

std::vector<Models::Pack> PackService::GetAllPacks() {
    return NStorage::GetAllPacks(pg_cluster_);
}

} // namespace game_userver::services
