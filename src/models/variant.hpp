#pragma once

#include <boost/uuid/uuid.hpp>
#include <string>

#include <userver/formats/json/value.hpp>
#include <userver/storages/postgres/io/row_types.hpp>

namespace NModels {

struct Variant final {
    boost::uuids::uuid id;
    boost::uuids::uuid pack_id;
    std::string text;
    std::string image_url;

    auto Introspect() const;
};

userver::formats::json::Value Serialize(
      const Variant& variant
    , userver::formats::serialize::To<userver::formats::json::Value>
);

} // NModels



namespace userver::storages::postgres::io {

template <>
struct CppToUserPg<NModels::Variant> {
    static constexpr DBTypeName postgres_name{"quiz.variant"};
};

} // namespace userver::storages::postgres::io
