#pragma once

#include <boost/uuid/uuid.hpp>
#include <chrono>

#include <userver/formats/json/value.hpp>
#include <userver/storages/postgres/io/row_types.hpp>

namespace NModels {

struct PlayerAnswer final {
    boost::uuids::uuid id;
    boost::uuids::uuid player_id;
    boost::uuids::uuid question_id;
    boost::uuids::uuid variant_id;
    bool is_correct;
    std::chrono::system_clock::time_point answered_at;

    auto Introspect() const;
};

userver::formats::json::Value Serialize(
      const PlayerAnswer& player_answer
    , userver::formats::serialize::To<userver::formats::json::Value>
);

PlayerAnswer Parse(
      const userver::formats::json::Value& json
    , userver::formats::parse::To<PlayerAnswer>
);

} // NModels

namespace userver::storages::postgres::io {

template <>
struct CppToUserPg<NModels::PlayerAnswer> {
    static constexpr DBTypeName postgres_name{"quiz.player_answer"};
};

} // namespace userver::storages::postgres::io