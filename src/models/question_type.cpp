#include "question_type.hpp"

#include <userver/utils/trivial_map.hpp>

namespace Models {

namespace {

constexpr userver::utils::TrivialBiMap kQuestionTypeMap = [](auto selector) -> auto {
    return selector()
        .Case("multiple_choice", QuestionType::kMultipleChoice)
        .Case("free_text", QuestionType::kFreeText)
        .Case("custom", QuestionType::kCustom);
};

} // namespace

auto ToString(QuestionType type) -> std::string {
    return std::string{kQuestionTypeMap.TryFindBySecond(type).value()};
}

auto ParseQuestionType(std::string_view str) -> QuestionType {
    auto result = kQuestionTypeMap.TryFindByFirst(str);
    if (!result) {
        throw std::invalid_argument(
            "Unknown question type: " + std::string(str)
        );
    }
    return *result;
}

} // namespace Models
