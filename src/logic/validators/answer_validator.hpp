#pragma once

#include <boost/uuid/uuid.hpp>
#include <memory>
#include <optional>
#include <string>

#include <userver/storages/postgres/cluster.hpp>

#include "models/player_answer.hpp"
#include "models/question.hpp"

namespace game_userver::logic::validators {

using userver::storages::postgres::ClusterPtr;

// Структура для передачи ответа игрока
struct PlayerAnswerInput {
    std::optional<boost::uuids::uuid> variant_id; // Для multiple_choice
    std::optional<std::string> text_answer;       // Для free_text
};

// Результат валидации
struct ValidationResult {
    bool is_correct;
    std::string message; // Опциональное сообщение для пользователя
};

// Интерфейс валидатора ответов
class IAnswerValidator {
public:
    virtual ~IAnswerValidator() = default;

    // Проверить правильность ответа
    [[nodiscard]] virtual auto Validate(
        const boost::uuids::uuid& question_id,
        const PlayerAnswerInput& player_input
    ) -> ValidationResult = 0;
};

// Валидатор для вопросов с выбором варианта
class MultipleChoiceValidator final : public IAnswerValidator {
public:
    explicit MultipleChoiceValidator(ClusterPtr pg_cluster);

    [[nodiscard]] auto Validate(
        const boost::uuids::uuid& question_id,
        const PlayerAnswerInput& player_input
    ) -> ValidationResult override;

private:
    ClusterPtr pg_cluster_;
};

// Валидатор для вопросов со свободным вводом текста
class FreeTextValidator final : public IAnswerValidator {
public:
    explicit FreeTextValidator(ClusterPtr pg_cluster);

    [[nodiscard]] auto Validate(
        const boost::uuids::uuid& question_id,
        const PlayerAnswerInput& player_input
    ) -> ValidationResult override;

private:
    ClusterPtr pg_cluster_;

    // Normalize text for comparison (lowercase, trim)
    [[nodiscard]] static auto NormalizeText(const std::string& text)
        -> std::string;
};

} // namespace game_userver::logic::validators
