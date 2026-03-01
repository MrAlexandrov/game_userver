#pragma once

#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/result_set.hpp>

#include "models/question.hpp"

namespace NStorage {

using userver::storages::postgres::ClusterPtr;
using userver::storages::postgres::ResultSet;

auto CreateQuestion(ClusterPtr pg_cluster_, Models::Question&& question)
    -> std::optional<Models::Question>;

auto GetQuestionById(
    ClusterPtr pg_cluster_, const Models::Question::QuestionId& question_id
) -> std::optional<Models::Question>;

auto GetQuestionsByPackId(
    ClusterPtr pg_cluster_, const Models::Pack::PackId& pack_id
) -> std::vector<Models::Question>;

} // namespace NStorage
