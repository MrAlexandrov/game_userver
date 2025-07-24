#include "question.hpp"
#include "utils/string_to_uuid.hpp"

#include <boost/uuid/uuid_io.hpp>
#include <userver/formats/json/value_builder.hpp>

namespace NModels {

auto Question::Introspect() const {
    return std::tie(id, pack_id, text, image_url);
}

userver::formats::json::Value Serialize(
      const Question& question
    , userver::formats::serialize::To<userver::formats::json::Value>
) {
    userver::formats::json::ValueBuilder item;
    item["id"]              = boost::uuids::to_string(question.id);
    item["pack_id"]         = boost::uuids::to_string(question.pack_id);
    item["text"]            = question.text;
    item["image_url"]       = question.image_url;
    return item.ExtractValue();
}

Question Parse(
      const userver::formats::json::Value& json
    , userver::formats::parse::To<Question>
) {
    Question question;
    question.id             = NUtils::StringToUuid(json["id"].As<std::string>());
    question.pack_id        = NUtils::StringToUuid(json["pack_id"].As<std::string>());
    question.text           = json["text"].As<std::string>();
    question.image_url      = json["image_url"].As<std::string>();
    return question;
}

} // namespace NModels
