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
    ClusterPtr pg_cluster_, const Models::Player::PlayerId& player_id,
    const Models::Question::QuestionId& question_id,
    const std::optional<Models::Variant::VariantId>& variant_id,
    const std::optional<std::string>& text_answer, bool is_correct
) -> std::optional<Models::PlayerAnswer>;

auto GetPlayerAnswersByPlayerId(
    ClusterPtr pg_cluster_, const Models::Player::PlayerId& player_id
) -> std::vector<Models::PlayerAnswer>;

auto CheckVariantCorrectnessById(
    ClusterPtr pg_cluster_, const Models::Variant::VariantId& variant_id
) -> std::optional<bool>;

auto GetAnswersCountForQuestion(
    ClusterPtr pg_cluster_,
    const Models::GameSession::GameSessionId& game_session_id,
    const Models::Question::QuestionId& question_id
) -> int;

} // namespace NStorage
