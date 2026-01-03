#pragma once

#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/result_set.hpp>

#include "models/question.hpp"

namespace NStorage {

using userver::storages::postgres::ClusterPtr;
using userver::storages::postgres::ResultSet;
using QuestionData = Models::Question::QuestionData;

auto CreateQuestion(ClusterPtr pg_cluster_, QuestionData&& data)
    -> std::optional<Models::Question>;

auto GetQuestionById(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& question_id
) -> std::optional<Models::Question>;

auto GetQuestionsByPackId(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& pack_id
) -> std::vector<Models::Question>;

} // namespace NStorage
