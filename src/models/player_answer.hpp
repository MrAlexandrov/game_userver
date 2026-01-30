#pragma once

#include <boost/uuid/uuid.hpp> // NOLINT
#include <chrono>
#include <optional>
#include <string>

#include <userver/formats/json/value.hpp>
#include <userver/storages/postgres/io/row_types.hpp>

namespace Models {

struct PlayerAnswer final {
    boost::uuids::uuid id{};
    boost::uuids::uuid player_id{};
    boost::uuids::uuid question_id{};
    std::optional<boost::uuids::uuid> variant_id{}; // For multiple_choice
    std::optional<std::string> text_answer{};       // For free_text
    bool is_correct{};
    std::chrono::system_clock::time_point answered_at;

    [[nodiscard]] auto Introspect() const;
};

auto Serialize(
    const PlayerAnswer& player_answer,
    userver::formats::serialize::To<userver::formats::json::Value>
) -> userver::formats::json::Value;

auto Parse(
    const userver::formats::json::Value& json,
    userver::formats::parse::To<PlayerAnswer>
) -> PlayerAnswer;

} // namespace Models

namespace userver::storages::postgres::io {

template <> struct CppToUserPg<Models::PlayerAnswer> {
    static constexpr DBTypeName postgres_name{"quiz.player_answer"};
};

} // namespace userver::storages::postgres::io
