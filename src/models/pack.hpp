#pragma once

#include <boost/uuid/uuid.hpp>
#include <string>

#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/formats/serialize/common_containers.hpp>
#include <userver/storages/postgres/io/row_types.hpp>

namespace NModels {

struct Pack final {
    boost::uuids::uuid id;
    std::string title;

    auto Introspect() const;
};

userver::formats::json::Value
Serialize(const Pack& pack, userver::formats::serialize::To<userver::formats::json::Value>);

Pack Parse(const userver::formats::json::Value& json, userver::formats::parse::To<Pack>);

} // namespace NModels

namespace userver::storages::postgres::io {

template <> struct CppToUserPg<NModels::Pack> {
    static constexpr DBTypeName postgres_name{"quiz.pack"};
};

} // namespace userver::storages::postgres::io
