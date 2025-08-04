#include "service.hpp"

#include "storage/pasks.hpp" // for db request CreatePack

#include <boost/uuid/uuid_io.hpp> // for responce
#include <models/models.pb.h> // proto model Pack
#include <models/pack.hpp> // cpp model Pack
#include <userver/storages/postgres/component.hpp>
#include <utils/string_to_uuid.hpp>

namespace game_userver {

Service::Service(
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

Service::CreatePackResult Service::CreatePack(
      CallContext&
    , handlers::api::CreatePackRequest&& request
) {
    auto createdPackOpt = NStorage::CreatePack(pg_cluster_, request.title());

    if (!createdPackOpt.has_value()) {
        throw std::runtime_error("Failed to create pack");
    }
    auto createdPack = createdPackOpt.value();

    handlers::api::CreatePackResponse responce;
    auto mutualPack = responce.mutable_pack();
    mutualPack->set_id(boost::uuids::to_string(std::move(createdPack.id)));
    mutualPack->set_title(std::move(createdPack.title));
    return responce;
}

Service::GetPackByIdResult Service::GetPackById(
      CallContext&
    , handlers::api::GetPackByIdRequest&& request
) {
    auto getPackByIdOpt = NStorage::GetPackById(pg_cluster_, NUtils::StringToUuid(request.id()));

    if (!getPackByIdOpt.has_value()) {
        throw std::runtime_error("No such pack");
    }
    auto getPackById = getPackByIdOpt.value();

    handlers::api::GetPackByIdResponse responce;
    auto mutualPack = responce.mutable_pack();
    mutualPack->set_id(boost::uuids::to_string(std::move(getPackById.id)));
    mutualPack->set_title(std::move(getPackById.title));
    return responce;
}

Service::GetAllPacksResult Service::GetAllPacks(
      CallContext&
    , handlers::api::GetAllPacksRequest&& request
) {
    auto getAllPacks = NStorage::GetAllPacks(pg_cluster_);

    handlers::api::GetAllPacksResponse responce;
    auto mutualPacks = responce.mutable_packs();
    for (auto&& pack : getAllPacks) {
        NModels::Proto::Pack packResponse;
        packResponse.set_id(boost::uuids::to_string(pack.id));
        packResponse.set_title(std::move(pack.title));

        mutualPacks->Add(std::move(packResponse));
    }
    return responce;
}

} // namespace game_userver
