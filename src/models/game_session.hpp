#pragma once

#include <boost/uuid/uuid.hpp>
#include <string>
#include <chrono>

#include <userver/formats/json/value.hpp>
#include <userver/storages/postgres/io/row_types.hpp>

namespace NModels {

struct GameSession final {
    boost::uuids::uuid id;
    boost::uuids::uuid pack_id;
    std::string state;  // waiting, active, finished
    int current_question_index;
    std::chrono::system_clock::time_point created_at;
    std::optional<std::chrono::system_clock::time_point> started_at;
    std::optional<std::chrono::system_clock::time_point> finished_at;

    auto Introspect() const;
};

userver::formats::json::Value Serialize(
      const GameSession& game_session
    , userver::formats::serialize::To<userver::formats::json::Value>
);

GameSession Parse(
      const userver::formats::json::Value& json
    , userver::formats::parse::To<GameSession>
);

} // NModels

namespace userver::storages::postgres::io {

template <>
struct CppToUserPg<NModels::GameSession> {
    static constexpr DBTypeName postgres_name{"quiz.game_session"};
};

} // namespace userver::storages::postgres::io