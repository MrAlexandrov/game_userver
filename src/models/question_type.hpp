#pragma once

#include <string>
#include <string_view>

namespace Models {

enum class QuestionType {
    kMultipleChoice,
    kFreeText,
    kCustom
};

// Convert QuestionType to string (for DB)
[[nodiscard]] auto ToString(QuestionType type) -> std::string;

// Parse string to QuestionType (from DB)
[[nodiscard]] auto ParseQuestionType(std::string_view str) -> QuestionType;

} // namespace Models
