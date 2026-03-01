#include "text_answers.hpp"

#include <sql_queries/sql_queries.hpp>
#include <userver/storages/postgres/cluster_types.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/io/io_fwd.hpp>

#include "utils/string_to_uuid.hpp"

namespace NStorage {

using userver::storages::postgres::ClusterPtr;
using namespace sql_queries::sql;
using userver::storages::postgres::ClusterHostType::kMaster;
using userver::storages::postgres::ClusterHostType::kSlave;

auto CreateTextAnswer(
    ClusterPtr pg_cluster_, const Models::TextAnswer& text_answer
) -> std::optional<Models::TextAnswer> {
    auto result = pg_cluster_->Execute(
        kMaster, kCreateTextAnswer, text_answer.question_id, text_answer.text
    );
    return result.AsOptionalSingleRow<Models::TextAnswer>(
        userver::storages::postgres::kRowTag
    );
}

auto GetTextAnswerById(
    ClusterPtr pg_cluster_,
    const Models::TextAnswer::TextAnswerId& text_answer_id
) -> std::optional<Models::TextAnswer> {
    auto result =
        pg_cluster_->Execute(kSlave, kGetTextAnswerById, text_answer_id);
    return result.AsOptionalSingleRow<Models::TextAnswer>(
        userver::storages::postgres::kRowTag
    );
}

auto GetTextAnswersByQuestionId(
    ClusterPtr pg_cluster_, const Models::Question::QuestionId& question_id
) -> std::vector<Models::TextAnswer> {
    auto result =
        pg_cluster_->Execute(kSlave, kGetTextAnswersByQuestionId, question_id);
    return result.AsContainer<std::vector<Models::TextAnswer>>(
        userver::storages::postgres::kRowTag
    );
}

} // namespace NStorage
