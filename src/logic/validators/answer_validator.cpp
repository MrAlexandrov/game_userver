#include "answer_validator.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>

#include "storage/text_answers.hpp"
#include "storage/variants.hpp"

namespace game_userver::logic::validators {

// MultipleChoiceValidator implementation

MultipleChoiceValidator::MultipleChoiceValidator(ClusterPtr pg_cluster)
    : pg_cluster_(std::move(pg_cluster)) {}

auto MultipleChoiceValidator::Validate(
    const boost::uuids::uuid& question_id, const PlayerAnswerInput& player_input
) -> ValidationResult {
    if (!player_input.variant_id.has_value()) {
        return ValidationResult{
            .is_correct = false,
            .message = "Variant ID is required for multiple choice questions"
        };
    }

    // Get the variant from database
    auto variant_opt =
        NStorage::GetVariantById(pg_cluster_, *player_input.variant_id);

    if (!variant_opt.has_value()) {
        return ValidationResult{
            .is_correct = false, .message = "Variant not found"
        };
    }

    // Check if variant belongs to the question
    if (variant_opt->question_id != question_id) {
        return ValidationResult{
            .is_correct = false,
            .message = "Variant does not belong to this question"
        };
    }

    return ValidationResult{
        .is_correct = variant_opt->is_correct,
        .message = variant_opt->is_correct ? "Correct!" : "Incorrect"
    };
}

// FreeTextValidator implementation

FreeTextValidator::FreeTextValidator(ClusterPtr pg_cluster)
    : pg_cluster_(std::move(pg_cluster)) {}

auto FreeTextValidator::Validate(
    const boost::uuids::uuid& question_id, const PlayerAnswerInput& player_input
) -> ValidationResult {
    if (!player_input.text_answer.has_value()) {
        return ValidationResult{
            .is_correct = false,
            .message = "Text answer is required for free text questions"
        };
    }

    // Normalize player's answer
    auto normalized_player_answer = NormalizeText(*player_input.text_answer);

    // Get correct answers from database
    auto correct_answers =
        NStorage::GetTextAnswersByQuestionId(pg_cluster_, question_id);

    if (correct_answers.empty()) {
        return ValidationResult{
            .is_correct = false,
            .message = "No correct answers configured for this question"
        };
    }

    // Check if player's answer matches any correct answer
    for (const auto& correct_answer : correct_answers) {
        auto normalized_correct = NormalizeText(correct_answer.text);
        if (normalized_player_answer == normalized_correct) {
            return ValidationResult{.is_correct = true, .message = "Correct!"};
        }
    }

    return ValidationResult{.is_correct = false, .message = "Incorrect"};
}

auto FreeTextValidator::NormalizeText(const std::string& text) -> std::string {
    std::string normalized = text;

    // Trim leading/trailing whitespace
    normalized.erase(0, normalized.find_first_not_of(" \t\n\r"));
    normalized.erase(normalized.find_last_not_of(" \t\n\r") + 1);

    // Convert to lowercase
    std::transform(
        normalized.begin(), normalized.end(), normalized.begin(),
        [](unsigned char c) {
            return std::tolower(c);
        }
    );

    return normalized;
}

} // namespace game_userver::logic::validators
