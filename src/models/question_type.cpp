#include "question_type.hpp"

#include <userver/formats/json/value_builder.hpp>

namespace Models {

std::string ToString(const QuestionType& questionType) {
    std::string type;
    const auto& enumerators =
        userver::storages::postgres::io::CppToUserPg<QuestionType>()
            .enumerators;
    for (const auto& [literal, enumerator] : enumerators) {
        if (literal == questionType) {
            type = std::string{enumerator.data(), enumerator.size()};
            break;
        }
    }
    return type;
}

QuestionType Parse(
    const userver::formats::json::Value& value,
    userver::formats::parse::To<QuestionType>
    /*unused*/
) {
    const auto& stringValue = value.As<std::string>();
    const auto& enumerators =
        userver::storages::postgres::io::CppToUserPg<QuestionType>()
            .enumerators;
    for (const auto& [literal, enumerator] : enumerators) {
        if (enumerator == stringValue) {
            return literal;
        }
    }

    throw userver::formats::json::ParseException(
        "Value of '" + value.GetPath() + "' (" + stringValue +
        ") is not parsable to QuestionType enum"
    );
}

userver::formats::json::Value Serialize(
    const QuestionType& questionType,
    userver::formats::serialize::To<userver::formats::json::Value>
) {
    return userver::formats::json::ValueBuilder(ToString(questionType))
        .ExtractValue();
}

} // namespace Models
