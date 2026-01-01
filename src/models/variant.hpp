#pragma once

#include <boost/uuid/uuid.hpp>
#include <string>
#include <userver/formats/json/value.hpp>
#include <userver/storages/postgres/io/row_types.hpp>

namespace Models {

struct Variant final {
    boost::uuids::uuid id;
    boost::uuids::uuid question_id;
    std::string text;
    bool is_correct;

    auto Introspect() const;
};

userver::formats::json::Value Serialize(
    const Variant& variant,
    userver::formats::serialize::To<userver::formats::json::Value>
);

Variant Parse(
    const userver::formats::json::Value& json,
    userver::formats::parse::To<Variant>
);

} // namespace Models

namespace userver::storages::postgres::io {

template <> struct CppToUserPg<Models::Variant> {
    static constexpr DBTypeName postgres_name{"quiz.variant"};
};

} // namespace userver::storages::postgres::io
