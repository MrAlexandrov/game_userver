#pragma once

#include <boost/uuid/uuid.hpp>
#include <chrono>
#include <string>

#include <userver/formats/json/value.hpp>
#include <userver/storages/postgres/io/row_types.hpp>

namespace Models {

struct Player final {
    boost::uuids::uuid id;
    boost::uuids::uuid game_session_id;
    std::string name;
    int score;
    std::chrono::system_clock::time_point joined_at;

    [[nodiscard]] auto Introspect() const;
};

auto Serialize(
    const Player& player,
    userver::formats::serialize::To<userver::formats::json::Value>
) -> userver::formats::json::Value;

auto Parse(
    const userver::formats::json::Value& json,
    userver::formats::parse::To<Player>
) -> Player;

} // namespace Models

namespace userver::storages::postgres::io {

template <> struct CppToUserPg<Models::Player> {
    static constexpr DBTypeName postgres_name{"quiz.player"};
};

} // namespace userver::storages::postgres::io
