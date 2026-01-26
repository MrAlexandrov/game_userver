#include "validator_factory.hpp"

#include <stdexcept>

namespace game_userver::logic::validators {

ValidatorFactory::ValidatorFactory(ClusterPtr pg_cluster)
    : pg_cluster_(std::move(pg_cluster)) {}

auto ValidatorFactory::CreateValidator(Models::QuestionType question_type)
    -> std::unique_ptr<IAnswerValidator> {
    switch (question_type) {
    case Models::QuestionType::kMultipleChoice:
        return std::make_unique<MultipleChoiceValidator>(pg_cluster_);

    case Models::QuestionType::kFreeText:
        return std::make_unique<FreeTextValidator>(pg_cluster_);

    case Models::QuestionType::kCustom:
        throw std::runtime_error(
            "Custom question type requires custom validator implementation"
        );

    default:
        throw std::invalid_argument("Unknown question type");
    }
}

} // namespace game_userver::logic::validators
