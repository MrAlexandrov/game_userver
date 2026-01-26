#pragma once

#include <memory>

#include <userver/storages/postgres/cluster.hpp>

#include "answer_validator.hpp"
#include "models/question_type.hpp"

namespace game_userver::logic::validators {

using userver::storages::postgres::ClusterPtr;

// Фабрика валидаторов
class ValidatorFactory final {
public:
    explicit ValidatorFactory(ClusterPtr pg_cluster);

    // Создать валидатор по типу вопроса
    [[nodiscard]] auto CreateValidator(Models::QuestionType question_type)
        -> std::unique_ptr<IAnswerValidator>;

private:
    ClusterPtr pg_cluster_;
};

} // namespace game_userver::logic::validators
