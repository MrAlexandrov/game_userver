#pragma once

#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/result_set.hpp>

#include "models/player_answer.hpp"

namespace NStorage {

// Wrapper struct for boolean values to work with userver
struct BooleanResult {
    bool value;

    [[nodiscard]] auto Introspect() const;
};

using userver::storages::postgres::ClusterPtr;
using userver::storages::postgres::ResultSet;

auto SubmitPlayerAnswer(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& player_id,
    const boost::uuids::uuid& question_id, const boost::uuids::uuid& variant_id,
    bool is_correct
) -> std::optional<Models::PlayerAnswer>;

auto GetPlayerAnswersByPlayerId(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& player_id
) -> std::vector<Models::PlayerAnswer>;

auto CheckVariantCorrectnessById(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& variant_id
) -> std::optional<bool>;

auto GetAnswersCountForQuestion(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& game_session_id,
    const boost::uuids::uuid& question_id
) -> int;

} // namespace NStorage
