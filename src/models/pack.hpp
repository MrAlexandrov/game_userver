#pragma once

#include <boost/uuid/uuid.hpp>
#include <string>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include <userver/storages/postgres/io/row_types.hpp>

namespace Models {

struct Pack final {
    struct PackData {
        std::string title;
    };

    boost::uuids::uuid id{};
    PackData data;

    [[nodiscard]] auto Introspect() const;
};

auto Serialize(
    const Pack& pack,
    userver::formats::serialize::To<userver::formats::json::Value>
) -> userver::formats::json::Value;

auto Parse(
    const userver::formats::json::Value& json, userver::formats::parse::To<Pack>
) -> Pack;

} // namespace Models

namespace userver::storages::postgres::io {

template <> struct CppToUserPg<Models::Pack> {
    static constexpr DBTypeName postgres_name{"quiz.pack"};
};

} // namespace userver::storages::postgres::io
