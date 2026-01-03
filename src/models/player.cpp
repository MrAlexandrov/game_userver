#include "player.hpp"
#include "utils/string_to_uuid.hpp"

#include <boost/uuid/uuid_io.hpp>
#include <userver/formats/json/value_builder.hpp>

namespace Models {

auto Player::Introspect() const {
    return std::tie(id, game_session_id, name, score, joined_at);
}

auto Serialize(
    const Player& player,
    userver::formats::serialize::To<userver::formats::json::Value>
    /*unused*/
) -> userver::formats::json::Value {
    userver::formats::json::ValueBuilder item;
    item["id"] = boost::uuids::to_string(player.id);
    item["game_session_id"] = boost::uuids::to_string(player.game_session_id);
    item["name"] = player.name;
    item["score"] = player.score;

    // Convert time point to string
    const auto joined_at_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            player.joined_at.time_since_epoch()
        )
            .count();
    item["joined_at"] = joined_at_ms;

    return item.ExtractValue();
}

auto Parse(
    const userver::formats::json::Value& json,
    userver::formats::parse::To<Player>
    /*unused*/
) -> Player {
    Player player;
    player.id = Utils::StringToUuid(json["id"].As<std::string>());
    player.game_session_id =
        Utils::StringToUuid(json["game_session_id"].As<std::string>());
    player.name = json["name"].As<std::string>();
    player.score = json["score"].As<int>();

    // Convert timestamp from milliseconds
    const auto joined_at_ms = json["joined_at"].As<long long>();
    player.joined_at = std::chrono::system_clock::time_point(
        std::chrono::milliseconds(joined_at_ms)
    );

    return player;
}

} // namespace Models
