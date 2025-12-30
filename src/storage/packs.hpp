#pragma once

#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/result_set.hpp>

#include "models/pack.hpp"

namespace NStorage {

using userver::storages::postgres::ClusterPtr;
using userver::storages::postgres::ResultSet;

auto CreatePack(ClusterPtr pg_cluster_, const std::string& title)
    -> std::optional<NModels::Pack>;

auto GetPackById(ClusterPtr pg_cluster_, const boost::uuids::uuid& pack_id)
    -> std::optional<NModels::Pack>;

auto GetAllPacks(ClusterPtr pg_cluster_) -> std::vector<NModels::Pack>;

} // namespace NStorage
