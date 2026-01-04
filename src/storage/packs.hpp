#pragma once

#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/result_set.hpp>

#include "models/pack.hpp"

namespace NStorage {

using userver::storages::postgres::ClusterPtr;
using userver::storages::postgres::ResultSet;

auto CreatePack(ClusterPtr pg_cluster_, const Models::Pack& pack)
    -> std::optional<Models::Pack>;

auto GetPackById(ClusterPtr pg_cluster_, const boost::uuids::uuid& pack_id)
    -> std::optional<Models::Pack>;

auto GetAllPacks(ClusterPtr pg_cluster_) -> std::vector<Models::Pack>;

} // namespace NStorage
