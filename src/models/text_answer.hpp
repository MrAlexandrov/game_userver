#pragma once

#include <boost/uuid/uuid.hpp>
#include <chrono>
#include <string>
#include <userver/formats/json/value.hpp>
#include <userver/storages/postgres/io/row_types.hpp>

namespace Models {

struct TextAnswer final {
    boost::uuids::uuid id{};
    boost::uuids::uuid question_id;
    std::string text;
    std::chrono::system_clock::time_point created_at;

    [[nodiscard]] auto Introspect() const;
};

auto Serialize(
    const TextAnswer& text_answer,
    userver::formats::serialize::To<userver::formats::json::Value>
) -> userver::formats::json::Value;

auto Parse(
    const userver::formats::json::Value& json,
    userver::formats::parse::To<TextAnswer>
) -> TextAnswer;

} // namespace Models

namespace userver::storages::postgres::io {

template <> struct CppToUserPg<Models::TextAnswer> {
    static constexpr DBTypeName postgres_name{"quiz.text_answer"};
};

} // namespace userver::storages::postgres::io
