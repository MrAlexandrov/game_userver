#pragma once

#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/result_set.hpp>

#include "models/variant.hpp"

namespace NStorage {

using userver::storages::postgres::ClusterPtr;
using userver::storages::postgres::ResultSet;

auto CreateVariant(
    ClusterPtr pg_cluster_, const Models::Variant::VariantData& data
) -> std::optional<Models::Variant>;

auto GetVariantById(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& variant_id
) -> std::optional<Models::Variant>;

auto GetVariantsByQuestionId(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& question_id
) -> std::vector<Models::Variant>;

} // namespace NStorage
