#include "questions.hpp"
#include "models/question.hpp"

#include <sql_queries/sql_queries.hpp>
#include <userver/storages/postgres/cluster_types.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/io/io_fwd.hpp>

namespace NStorage {

using userver::storages::postgres::ClusterPtr;
using namespace sql_queries::sql;
using userver::storages::postgres::ClusterHostType::kMaster;
using userver::storages::postgres::ClusterHostType::kSlave;

auto CreateQuestion(ClusterPtr pg_cluster_, Utils::QuestionData&& data)
    -> std::optional<Models::Question> {
    auto result = pg_cluster_->Execute(
        kMaster, kCreateQuestion, data.pack_id, data.text, data.image_url
    );
    return result.AsOptionalSingleRow<Models::Question>(
        userver::storages::postgres::kRowTag
    );
}

auto GetQuestionById(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& question_id
) -> std::optional<Models::Question> {
    auto result = pg_cluster_->Execute(kSlave, kGetQuestionById, question_id);
    return result.AsOptionalSingleRow<Models::Question>(
        userver::storages::postgres::kRowTag
    );
}

auto GetQuestionsByPackId(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& pack_id
) -> std::vector<Models::Question> {
    auto result = pg_cluster_->Execute(kSlave, kGetQuestionsByPackId, pack_id);
    return result.AsContainer<std::vector<Models::Question>>(
        userver::storages::postgres::kRowTag
    );
}

} // namespace NStorage
