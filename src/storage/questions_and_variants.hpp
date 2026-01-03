#pragma once

#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/result_set.hpp>

#include "models/question.hpp"
#include "models/variant.hpp"

namespace NStorage {

using userver::storages::postgres::ClusterPtr;
using userver::storages::postgres::ResultSet;

auto GetQuestionsAndVariantsByPackId(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& pack_id
) -> std::vector<std::pair<Models::Question, std::vector<Models::Variant>>>;

} // namespace NStorage
