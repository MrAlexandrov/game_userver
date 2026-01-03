#include "question.hpp"

#include <boost/uuid/uuid_io.hpp>
#include <userver/formats/json/value_builder.hpp>

#include "utils/string_to_uuid.hpp"

namespace Models {

auto Question::Introspect() const {
    return std::tie(id, data.pack_id, data.text, data.image_url);
}

auto Serialize(
    const Question& question,
    userver::formats::serialize::To<userver::formats::json::Value>
    /*unused*/
) -> userver::formats::json::Value {
    userver::formats::json::ValueBuilder item;
    item["id"] = boost::uuids::to_string(question.id);
    item["pack_id"] = boost::uuids::to_string(question.data.pack_id);
    item["text"] = question.data.text;
    item["image_url"] = question.data.image_url;
    return item.ExtractValue();
}

auto Parse(
    const userver::formats::json::Value& json,
    userver::formats::parse::To<Question>
    /*unused*/
) -> Question {
    return Question{
        .id = Utils::StringToUuid(json["id"].As<std::string>()),
        .data = {
                 .pack_id = Utils::StringToUuid(json["pack_id"].As<std::string>()),
                 .text = json["text"].As<std::string>(),
                 .image_url = json["image_url"].As<std::string>(),
                 },
    };
}

} // namespace Models
