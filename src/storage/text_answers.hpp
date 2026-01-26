#pragma once

#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/result_set.hpp>

#include "models/text_answer.hpp"

namespace NStorage {

using userver::storages::postgres::ClusterPtr;
using userver::storages::postgres::ResultSet;

auto CreateTextAnswer(
    ClusterPtr pg_cluster_, const Models::TextAnswer& text_answer
) -> std::optional<Models::TextAnswer>;

auto GetTextAnswerById(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& text_answer_id
) -> std::optional<Models::TextAnswer>;

auto GetTextAnswersByQuestionId(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& question_id
) -> std::vector<Models::TextAnswer>;

} // namespace NStorage
