#pragma once

#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/result_set.hpp>

#include "models/question.hpp"

namespace NStorage {

using userver::storages::postgres::ClusterPtr;
using userver::storages::postgres::ResultSet;

auto CreateQuestion(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& pack_id,
    const std::string& text, const std::string& image_url
) -> std::optional<NModels::Question>;

auto GetQuestionById(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& question_id
) -> std::optional<NModels::Question>;

auto GetQuestionsByPackId(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& pack_id
) -> std::vector<NModels::Question>;

} // namespace NStorage
