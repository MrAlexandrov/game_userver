#pragma once

#include "models/player.hpp"

#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/result_set.hpp>

namespace NStorage {

using userver::storages::postgres::ClusterPtr;
using userver::storages::postgres::ResultSet;

auto AddPlayer(ClusterPtr pg_cluster_, const boost::uuids::uuid& game_session_id, const std::string& name) -> std::optional<NModels::Player>;

auto GetPlayersByGameSessionId(ClusterPtr pg_cluster_, const boost::uuids::uuid& game_session_id) -> std::vector<NModels::Player>;

auto UpdatePlayerScore(ClusterPtr pg_cluster_, const boost::uuids::uuid& player_id, int points) -> std::optional<NModels::Player>;

} // namespace NStorage