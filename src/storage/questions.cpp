#include "questions.hpp"

#include <userver/storages/postgres/cluster_types.hpp>
#include <userver/storages/postgres/component.hpp>

#include <samples_postgres_service/sql_queries.hpp>

#include <userver/storages/postgres/io/io_fwd.hpp>

namespace NStorage {

using userver::storages::postgres::ClusterPtr;
using namespace samples_postgres_service::sql;
using userver::storages::postgres::ClusterHostType::kMaster;
using userver::storages::postgres::ClusterHostType::kSlave;

auto CreateQuestion(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& pack_id,
    const std::string& text, const std::string& image_url
) -> std::optional<NModels::Question> {
    auto result = pg_cluster_->Execute(
        kMaster, kCreateQuestion, pack_id, text, image_url
    );
    return result.AsOptionalSingleRow<NModels::Question>(
        userver::storages::postgres::kRowTag
    );
}

auto GetQuestionById(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& question_id
) -> std::optional<NModels::Question> {
    auto result = pg_cluster_->Execute(kSlave, kGetQuestionById, question_id);
    return result.AsOptionalSingleRow<NModels::Question>(
        userver::storages::postgres::kRowTag
    );
}

auto GetQuestionsByPackId(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& pack_id
) -> std::vector<NModels::Question> {
    auto result = pg_cluster_->Execute(kSlave, kGetQuestionsByPackId, pack_id);
    return result.AsContainer<std::vector<NModels::Question>>(
        userver::storages::postgres::kRowTag
    );
}

} // namespace NStorage
