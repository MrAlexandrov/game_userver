#include "player_answers.hpp"

#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/cluster_types.hpp>

#include <samples_postgres_service/sql_queries.hpp>

#include <userver/storages/postgres/io/io_fwd.hpp>

namespace NStorage {

using userver::storages::postgres::ClusterPtr;
using namespace samples_postgres_service::sql;
using userver::storages::postgres::ClusterHostType::kMaster;
using userver::storages::postgres::ClusterHostType::kSlave;

auto SubmitPlayerAnswer(ClusterPtr pg_cluster_, const boost::uuids::uuid& player_id, const boost::uuids::uuid& question_id, const boost::uuids::uuid& variant_id, bool is_correct) -> std::optional<NModels::PlayerAnswer> {
    auto result = pg_cluster_->Execute(
        kMaster,
        kSubmitPlayerAnswer,
        player_id,
        question_id,
        variant_id,
        is_correct
    );
    return result.AsOptionalSingleRow<NModels::PlayerAnswer>(userver::storages::postgres::kRowTag);
}

auto GetPlayerAnswersByPlayerId(ClusterPtr pg_cluster_, const boost::uuids::uuid& player_id) -> std::vector<NModels::PlayerAnswer> {
    auto result = pg_cluster_->Execute(
        kSlave,
        kGetPlayerAnswersByPlayerId,
        player_id
    );
    return result.AsContainer<std::vector<NModels::PlayerAnswer>>(userver::storages::postgres::kRowTag);
}

} // namespace NStorage