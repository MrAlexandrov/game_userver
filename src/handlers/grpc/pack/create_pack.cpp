#include "create_pack.hpp"

#include "responce.hpp" // CreateResponse
#include "storage/pasks.hpp" // for db request CreatePack

#include <models/models.pb.h> // proto model Pack
#include <userver/storages/postgres/component.hpp>
#include <models/pack.hpp> // cpp model Pack

namespace game_userver {

CreatePackGrpc::CreatePackGrpc(
      const userver::components::ComponentConfig& config
    , const userver::components::ComponentContext& component_context
)
    : handlers::api::QuizServiceBase::Component(config, component_context)
    , pg_cluster_(
        component_context.FindComponent<userver::components::Postgres>("postgres-db-1")
            .GetCluster()
    )
{
}

CreatePackGrpc::CreatePackResult CreatePackGrpc::CreatePack(
      CallContext&
    , handlers::api::CreatePackRequest&& request
) {
    auto createdPackOpt = NStorage::CreatePack(pg_cluster_, request.title());

    if (!createdPackOpt.has_value()) {
        throw std::runtime_error("Failed to create pack");
    }

    return CreateResponse(createdPackOpt.value());
}

} // namespace game_userver
