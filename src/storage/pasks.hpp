#pragma once

#include "models/pack.hpp"

#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/result_set.hpp>

namespace NStorage {

using userver::storages::postgres::ClusterPtr;
using userver::storages::postgres::ResultSet;

auto CreatePack(ClusterPtr pg_cluster_, const std::string& title) -> std::optional<NModels::Pack>;

auto GetPack(ClusterPtr pg_cluster_, const boost::uuids::uuid& uuid) -> std::optional<NModels::Pack>;

auto GetAllPacks(ClusterPtr pg_cluster_) -> std::vector<NModels::Pack>;

} // namespace NStorage
