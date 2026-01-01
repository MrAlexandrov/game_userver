#include "variants.hpp"

#include <sql_queries/sql_queries.hpp>
#include <userver/storages/postgres/cluster_types.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/io/io_fwd.hpp>

namespace NStorage {

using userver::storages::postgres::ClusterPtr;
using namespace sql_queries::sql;
using userver::storages::postgres::ClusterHostType::kMaster;
using userver::storages::postgres::ClusterHostType::kSlave;

auto CreateVariant(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& question_id,
    const std::string& text, bool is_correct
) -> std::optional<NModels::Variant> {
    auto result = pg_cluster_->Execute(
        kMaster, kCreateVariant, question_id, text, is_correct
    );
    return result.AsOptionalSingleRow<NModels::Variant>(
        userver::storages::postgres::kRowTag
    );
}

auto GetVariantById(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& variant_id
) -> std::optional<NModels::Variant> {
    auto result = pg_cluster_->Execute(kSlave, kGetVariantById, variant_id);
    return result.AsOptionalSingleRow<NModels::Variant>(
        userver::storages::postgres::kRowTag
    );
}

auto GetVariantsByQuestionId(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& question_id
) -> std::vector<NModels::Variant> {
    auto result =
        pg_cluster_->Execute(kSlave, kGetVariantsByQuestionId, question_id);
    return result.AsContainer<std::vector<NModels::Variant>>(
        userver::storages::postgres::kRowTag
    );
}

} // namespace NStorage
