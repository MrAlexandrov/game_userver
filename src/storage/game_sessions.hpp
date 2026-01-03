#pragma once

#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/result_set.hpp>

#include "models/game_session.hpp"

namespace NStorage {

using userver::storages::postgres::ClusterPtr;
using userver::storages::postgres::ResultSet;

auto CreateGameSession(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& pack_id
) -> std::optional<Models::GameSession>;

auto GetGameSessionById(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& game_session_id
) -> std::optional<Models::GameSession>;

auto StartGameSession(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& game_session_id
) -> std::optional<Models::GameSession>;

auto AdvanceToNextQuestion(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& game_session_id,
    int current_question_index
) -> std::optional<Models::GameSession>;

auto EndGameSession(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& game_session_id
) -> std::optional<Models::GameSession>;

auto GetAllGameSessions(ClusterPtr pg_cluster_)
    -> std::vector<Models::GameSession>;

} // namespace NStorage
