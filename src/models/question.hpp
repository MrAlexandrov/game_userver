#pragma once

#include <boost/uuid/uuid.hpp>
#include <string>
#include <userver/formats/json/value.hpp>
#include <userver/storages/postgres/io/row_types.hpp>

namespace Models {

struct Question final {
    boost::uuids::uuid id;
    boost::uuids::uuid pack_id;
    std::string text;
    std::string image_url;

    [[nodiscard]] auto Introspect() const;
};

auto Serialize(
    const Question& question,
    userver::formats::serialize::To<userver::formats::json::Value>
) -> userver::formats::json::Value;

auto Parse(
    const userver::formats::json::Value& json,
    userver::formats::parse::To<Question>
) -> Question;

} // namespace Models

namespace userver::storages::postgres::io {

template <> struct CppToUserPg<Models::Question> {
    static constexpr DBTypeName postgres_name{"quiz.question"};
};

} // namespace userver::storages::postgres::io
