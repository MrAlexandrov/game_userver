#include "game_session.hpp"
#include "utils/string_to_uuid.hpp"

#include <boost/uuid/uuid_io.hpp>
#include <userver/formats/json/value_builder.hpp>

namespace Models {

auto GameSession::Introspect() const {
    return std::tie(
        id, pack_id, state, current_question_index, created_at, started_at,
        finished_at
    );
}

auto Serialize(
    const GameSession& game_session,
    userver::formats::serialize::To<userver::formats::json::Value>
    /*unused*/
) -> userver::formats::json::Value {
    userver::formats::json::ValueBuilder item;
    item["id"] = boost::uuids::to_string(game_session.id);
    item["pack_id"] = boost::uuids::to_string(game_session.pack_id);
    item["state"] = game_session.state;
    item["current_question_index"] = game_session.current_question_index;

    // Convert time points to strings
    const auto created_at_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            game_session.created_at.time_since_epoch()
        )
            .count();
    item["created_at"] = created_at_ms;

    if (game_session.started_at.has_value()) {
        const auto started_at_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                game_session.started_at.value().time_since_epoch()
            )
                .count();
        item["started_at"] = started_at_ms;
    }

    if (game_session.finished_at.has_value()) {
        const auto finished_at_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                game_session.finished_at.value().time_since_epoch()
            )
                .count();
        item["finished_at"] = finished_at_ms;
    }

    return item.ExtractValue();
}

auto Parse(
    const userver::formats::json::Value& json,
    userver::formats::parse::To<GameSession>
    /*unused*/
) -> GameSession {
    GameSession game_session;
    game_session.id = Utils::StringToUuid(json["id"].As<std::string>());
    game_session.pack_id =
        Utils::StringToUuid(json["pack_id"].As<std::string>());
    game_session.state = json["state"].As<std::string>();
    game_session.current_question_index =
        json["current_question_index"].As<int>();

    // Convert timestamps from milliseconds
    auto created_at_ms = json["created_at"].As<long long>();
    game_session.created_at = std::chrono::system_clock::time_point(
        std::chrono::milliseconds(created_at_ms)
    );

    if (json.HasMember("started_at")) {
        auto started_at_ms = json["started_at"].As<long long>();
        game_session.started_at = std::chrono::system_clock::time_point(
            std::chrono::milliseconds(started_at_ms)
        );
    }

    if (json.HasMember("finished_at")) {
        auto finished_at_ms = json["finished_at"].As<long long>();
        game_session.finished_at = std::chrono::system_clock::time_point(
            std::chrono::milliseconds(finished_at_ms)
        );
    }

    return game_session;
}

} // namespace Models
