#pragma once

#include <chrono>
#include <string>

#include <userver/formats/json/value.hpp>
#include <userver/storages/postgres/io/row_types.hpp>

#include "models/pack.hpp"

namespace Models {

struct GameSession final {
    using GameSessionId = boost::uuids::uuid;

    GameSessionId id;
    Pack::PackId pack_id;
    std::string state; // waiting, active, finished
    int current_question_index;
    std::chrono::system_clock::time_point created_at;
    std::optional<std::chrono::system_clock::time_point> started_at;
    std::optional<std::chrono::system_clock::time_point> finished_at;

    [[nodiscard]] auto Introspect() const;
};

auto Serialize(
    const GameSession& game_session,
    userver::formats::serialize::To<userver::formats::json::Value>
) -> userver::formats::json::Value;

auto Parse(
    const userver::formats::json::Value& json,
    userver::formats::parse::To<GameSession>
) -> GameSession;

} // namespace Models

namespace userver::storages::postgres::io {

template <> struct CppToUserPg<Models::GameSession> {
    static constexpr DBTypeName postgres_name{"quiz.game_session"};
};

} // namespace userver::storages::postgres::io
