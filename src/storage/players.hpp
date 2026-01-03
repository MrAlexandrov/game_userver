#pragma once

#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/result_set.hpp>

#include "models/player.hpp"

namespace NStorage {

using userver::storages::postgres::ClusterPtr;
using userver::storages::postgres::ResultSet;

auto AddPlayer(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& game_session_id,
    const std::string& name
) -> std::optional<Models::Player>;

auto GetPlayerById(ClusterPtr pg_cluster_, const boost::uuids::uuid& player_id)
    -> std::optional<Models::Player>;

auto GetPlayersByGameSessionId(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& game_session_id
) -> std::vector<Models::Player>;

auto UpdatePlayerScore(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& player_id,
    int score_increment
) -> std::optional<Models::Player>;

} // namespace NStorage
