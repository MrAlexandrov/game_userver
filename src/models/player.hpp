#pragma once

#include <boost/uuid/uuid.hpp>
#include <string>
#include <chrono>

#include <userver/formats/json/value.hpp>
#include <userver/storages/postgres/io/row_types.hpp>

namespace NModels {

struct Player final {
    boost::uuids::uuid id;
    boost::uuids::uuid game_session_id;
    std::string name;
    int score;
    std::chrono::system_clock::time_point joined_at;

    auto Introspect() const;
};

userver::formats::json::Value Serialize(
      const Player& player
    , userver::formats::serialize::To<userver::formats::json::Value>
);

Player Parse(
      const userver::formats::json::Value& json
    , userver::formats::parse::To<Player>
);

} // NModels

namespace userver::storages::postgres::io {

template <>
struct CppToUserPg<NModels::Player> {
    static constexpr DBTypeName postgres_name{"quiz.player"};
};

} // namespace userver::storages::postgres::io