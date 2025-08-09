#pragma once

#include "models/player_answer.hpp"

#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/result_set.hpp>

namespace NStorage {

using userver::storages::postgres::ClusterPtr;
using userver::storages::postgres::ResultSet;

auto SubmitPlayerAnswer(ClusterPtr pg_cluster_, const boost::uuids::uuid& player_id, const boost::uuids::uuid& question_id, const boost::uuids::uuid& variant_id, bool is_correct) -> std::optional<NModels::PlayerAnswer>;

auto GetPlayerAnswersByPlayerId(ClusterPtr pg_cluster_, const boost::uuids::uuid& player_id) -> std::vector<NModels::PlayerAnswer>;

} // namespace NStorage