#pragma once

#include <string>
#include <string_view>

#include <userver/formats/json/value.hpp>
#include <userver/storages/postgres/io/enum_types.hpp>
#include <userver/storages/postgres/io/io_fwd.hpp>

namespace Models {

enum class QuestionType {
    kMultipleChoice,
    kFreeText,
    kCustom
};

// Convert QuestionType to string
std::string ToString(const QuestionType& questionType);

// Parse from JSON
QuestionType Parse(
    const userver::formats::json::Value& value,
    userver::formats::parse::To<QuestionType>
);

// Serialize to JSON
userver::formats::json::Value Serialize(
    const QuestionType& questionType,
    userver::formats::serialize::To<userver::formats::json::Value>
);

} // namespace Models

// PostgreSQL enum mapping
namespace userver::storages::postgres::io {

template <>
struct CppToUserPg<Models::QuestionType>
    : EnumMappingBase<Models::QuestionType> {
    static constexpr DBTypeName postgres_name = "text";
    static constexpr EnumeratorList enumerators{
        {Models::QuestionType::kMultipleChoice, "multiple_choice"},
        {Models::QuestionType::kFreeText,       "free_text"      },
        {Models::QuestionType::kCustom,         "custom"         },
    };
};

} // namespace userver::storages::postgres::io
