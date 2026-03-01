#include "player_answers.hpp"

#include <sql_queries/sql_queries.hpp>
#include <userver/storages/postgres/cluster_types.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/io/io_fwd.hpp>

#include "models/player_answer.hpp"
#include "models/variant.hpp"

namespace NStorage {

using userver::storages::postgres::ClusterPtr;
using namespace sql_queries::sql;
using userver::storages::postgres::ClusterHostType::kMaster;
using userver::storages::postgres::ClusterHostType::kSlave;

auto SubmitPlayerAnswer(
    ClusterPtr pg_cluster_, const Models::Player::PlayerId& player_id,
    const Models::Question::QuestionId& question_id,
    const std::optional<Models::Variant::VariantId>& variant_id,
    const std::optional<std::string>& text_answer, bool is_correct
) -> std::optional<Models::PlayerAnswer> {
    auto result = pg_cluster_->Execute(
        kMaster, kSubmitPlayerAnswer, player_id, question_id, variant_id,
        text_answer, is_correct
    );
    return result.AsOptionalSingleRow<Models::PlayerAnswer>(
        userver::storages::postgres::kRowTag
    );
}

auto GetPlayerAnswersByPlayerId(
    ClusterPtr pg_cluster_, const Models::Player::PlayerId& player_id
) -> std::vector<Models::PlayerAnswer> {
    auto result =
        pg_cluster_->Execute(kSlave, kGetPlayerAnswersByPlayerId, player_id);
    return result.AsContainer<std::vector<Models::PlayerAnswer>>(
        userver::storages::postgres::kRowTag
    );
}

auto CheckVariantCorrectnessById(
    ClusterPtr pg_cluster_, const Models::Variant::VariantId& variant_id
) -> std::optional<bool> {
    auto result =
        pg_cluster_->Execute(kSlave, kCheckVariantCorrectnessById, variant_id);

    if (result.IsEmpty()) {
        return std::nullopt;
    }

    const auto row = result[0];
    return row["is_correct"].As<bool>();
}

auto GetAnswersCountForQuestion(
    ClusterPtr pg_cluster_,
    const Models::GameSession::GameSessionId& game_session_id,
    const Models::Question::QuestionId& question_id
) -> int {
    // Use kMaster to guarantee we see the answer just submitted in this request
    auto result = pg_cluster_->Execute(
        kMaster, kGetAnswersCountForQuestion, game_session_id, question_id
    );

    if (result.IsEmpty()) {
        return 0;
    }

    const auto row = result[0];
    return row["count"].As<int>();
}

} // namespace NStorage
