#include "packs.hpp"

#include <sql_queries/sql_queries.hpp>
#include <userver/storages/postgres/cluster_types.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/io/io_fwd.hpp>

#include "models/pack.hpp"

namespace NStorage {

using userver::storages::postgres::ClusterPtr;
using namespace sql_queries::sql;
using userver::storages::postgres::ClusterHostType::kMaster;
using userver::storages::postgres::ClusterHostType::kSlave;

auto CreatePack(ClusterPtr pg_cluster_, const Utils::PackData& data)
    -> std::optional<Models::Pack> {
    // TODO: handle empty title
    auto result = pg_cluster_->Execute(kMaster, kCreatePack, data.title);
    return result.AsOptionalSingleRow<Models::Pack>(
        userver::storages::postgres::kRowTag
    );
}

auto GetPackById(ClusterPtr pg_cluster_, const boost::uuids::uuid& pack_id)
    -> std::optional<Models::Pack> {
    auto result = pg_cluster_->Execute(kMaster, kGetPackById, pack_id);
    return result.AsOptionalSingleRow<Models::Pack>(
        userver::storages::postgres::kRowTag
    );
}

auto GetAllPacks(ClusterPtr pg_cluster_) -> std::vector<Models::Pack> {
    auto result = pg_cluster_->Execute(kSlave, kGetAllPacks);
    return result.AsContainer<std::vector<Models::Pack>>(
        userver::storages::postgres::kRowTag
    );
}

} // namespace NStorage
