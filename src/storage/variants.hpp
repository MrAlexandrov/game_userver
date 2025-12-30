#pragma once

#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/result_set.hpp>

#include "models/variant.hpp"

namespace NStorage {

using userver::storages::postgres::ClusterPtr;
using userver::storages::postgres::ResultSet;

auto CreateVariant(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& question_id,
    const std::string& text, bool is_correct
) -> std::optional<NModels::Variant>;

auto GetVariantById(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& variant_id
) -> std::optional<NModels::Variant>;

auto GetVariantsByQuestionId(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& question_id
) -> std::vector<NModels::Variant>;

} // namespace NStorage
