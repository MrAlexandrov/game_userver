#include "packs.hpp"

#include "models/pack.hpp"

#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/cluster_types.hpp>

#include <samples_postgres_service/sql_queries.hpp>

#include <userver/storages/postgres/io/io_fwd.hpp>

namespace NStorage {

using userver::storages::postgres::ClusterPtr;
using namespace samples_postgres_service::sql;
using userver::storages::postgres::ClusterHostType::kMaster;
using userver::storages::postgres::ClusterHostType::kSlave;

auto CreatePack(ClusterPtr pg_cluster_, const std::string& title) -> std::optional<NModels::Pack> {
    // TODO: handle empty title
    auto result = pg_cluster_->Execute(
        kMaster,
        kCreatePack,
        title
    );
    return result.AsOptionalSingleRow<NModels::Pack>(userver::storages::postgres::kRowTag);
}

auto GetPackById(ClusterPtr pg_cluster_, const boost::uuids::uuid& pack_id) -> std::optional<NModels::Pack> {
    auto result = pg_cluster_->Execute(
        kMaster,
        kGetPackById,
        pack_id
    );
    return result.AsOptionalSingleRow<NModels::Pack>(userver::storages::postgres::kRowTag);
}

auto GetAllPacks(ClusterPtr pg_cluster_) -> std::vector<NModels::Pack> {
    auto result = pg_cluster_->Execute(
        kSlave,
        kGetAllPacks
    );
    return result.AsContainer<std::vector<NModels::Pack>>(userver::storages::postgres::kRowTag);
}

} // namespace NStorage
