#include "variants.hpp"

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

auto CreateVariant(ClusterPtr pg_cluster_, const Models::Variant& variant)
    -> std::optional<Models::Variant> {
    auto result = pg_cluster_->Execute(
        kMaster, kCreateVariant, variant.question_id, variant.text,
        variant.is_correct
    );
    return result.AsOptionalSingleRow<Models::Variant>(
        userver::storages::postgres::kRowTag
    );
}

auto GetVariantById(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& variant_id
) -> std::optional<Models::Variant> {
    auto result = pg_cluster_->Execute(kSlave, kGetVariantById, variant_id);
    return result.AsOptionalSingleRow<Models::Variant>(
        userver::storages::postgres::kRowTag
    );
}

auto GetVariantsByQuestionId(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& question_id
) -> std::vector<Models::Variant> {
    auto result =
        pg_cluster_->Execute(kSlave, kGetVariantsByQuestionId, question_id);
    return result.AsContainer<std::vector<Models::Variant>>(
        userver::storages::postgres::kRowTag
    );
}

} // namespace NStorage
