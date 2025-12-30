#pragma once

#include <boost/uuid/uuid.hpp>
#include <string>

#include <userver/formats/json/value.hpp>
#include <userver/storages/postgres/io/row_types.hpp>

namespace NModels {

struct Question final {
    boost::uuids::uuid id;
    boost::uuids::uuid pack_id;
    std::string text;
    std::string image_url;

    auto Introspect() const;
};

userver::formats::json::Value Serialize(
    const Question& question,
    userver::formats::serialize::To<userver::formats::json::Value>
);

Question Parse(
    const userver::formats::json::Value& json,
    userver::formats::parse::To<Question>
);

} // namespace NModels

namespace userver::storages::postgres::io {

template <> struct CppToUserPg<NModels::Question> {
    static constexpr DBTypeName postgres_name{"quiz.question"};
};

} // namespace userver::storages::postgres::io
