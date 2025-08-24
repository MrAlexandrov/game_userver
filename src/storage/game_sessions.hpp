#pragma once

#include "models/game_session.hpp"

#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/result_set.hpp>

namespace NStorage {

using userver::storages::postgres::ClusterPtr;
using userver::storages::postgres::ResultSet;

auto CreateGameSession(ClusterPtr pg_cluster_, const boost::uuids::uuid& pack_id) -> std::optional<NModels::GameSession>;

auto GetGameSessionById(ClusterPtr pg_cluster_, const boost::uuids::uuid& game_session_id) -> std::optional<NModels::GameSession>;

auto StartGameSession(ClusterPtr pg_cluster_, const boost::uuids::uuid& game_session_id) -> std::optional<NModels::GameSession>;

auto EndGameSession(ClusterPtr pg_cluster_, const boost::uuids::uuid& game_session_id) -> std::optional<NModels::GameSession>;

auto GetAllGameSessions(ClusterPtr pg_cluster_) -> std::vector<NModels::GameSession>;

auto AdvanceToNextQuestion(ClusterPtr pg_cluster_, const boost::uuids::uuid& game_session_id) -> std::optional<NModels::GameSession>;

auto SetCurrentQuestionIndex(ClusterPtr pg_cluster_, const boost::uuids::uuid& game_session_id, int current_question_index) -> std::optional<NModels::GameSession>;

} // namespace NStorage