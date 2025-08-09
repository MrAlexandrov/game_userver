#include "player_answer.hpp"
#include "utils/string_to_uuid.hpp"

#include <boost/uuid/uuid_io.hpp>
#include <userver/formats/json/value_builder.hpp>

namespace NModels {

auto PlayerAnswer::Introspect() const {
    return std::tie(id, player_id, question_id, variant_id, is_correct, answered_at);
}

userver::formats::json::Value Serialize(
      const PlayerAnswer& player_answer
    , userver::formats::serialize::To<userver::formats::json::Value>
) {
    userver::formats::json::ValueBuilder item;
    item["id"] = boost::uuids::to_string(player_answer.id);
    item["player_id"] = boost::uuids::to_string(player_answer.player_id);
    item["question_id"] = boost::uuids::to_string(player_answer.question_id);
    item["variant_id"] = boost::uuids::to_string(player_answer.variant_id);
    item["is_correct"] = player_answer.is_correct;
    
    // Convert time point to string
    auto answered_at_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        player_answer.answered_at.time_since_epoch()).count();
    item["answered_at"] = answered_at_ms;
    
    return item.ExtractValue();
}

PlayerAnswer Parse(
      const userver::formats::json::Value& json
    , userver::formats::parse::To<PlayerAnswer>
) {
    PlayerAnswer player_answer;
    player_answer.id = NUtils::StringToUuid(json["id"].As<std::string>());
    player_answer.player_id = NUtils::StringToUuid(json["player_id"].As<std::string>());
    player_answer.question_id = NUtils::StringToUuid(json["question_id"].As<std::string>());
    player_answer.variant_id = NUtils::StringToUuid(json["variant_id"].As<std::string>());
    player_answer.is_correct = json["is_correct"].As<bool>();
    
    // Convert timestamp from milliseconds
    auto answered_at_ms = json["answered_at"].As<long long>();
    player_answer.answered_at = std::chrono::system_clock::time_point(
        std::chrono::milliseconds(answered_at_ms));
    
    return player_answer;
}

} // namespace NModels