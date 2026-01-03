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

    [[nodiscard]] auto Introspect() const;
};

auto Serialize(
    const Variant& variant,
    userver::formats::serialize::To<userver::formats::json::Value>
) -> userver::formats::json::Value;

auto Parse(
    const userver::formats::json::Value& json,
    userver::formats::parse::To<Variant>
) -> Variant;

} // namespace Models

namespace userver::storages::postgres::io {

template <> struct CppToUserPg<Models::Variant> {
    static constexpr DBTypeName postgres_name{"quiz.variant"};
};

} // namespace userver::storages::postgres::io
