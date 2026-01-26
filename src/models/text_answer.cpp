#include "text_answer.hpp"

#include <boost/uuid/uuid_io.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/utils/datetime.hpp>

#include "utils/string_to_uuid.hpp"

namespace Models {

auto TextAnswer::Introspect() const {
    return std::tie(id, question_id, text, created_at);
}

auto Serialize(
    const TextAnswer& text_answer,
    userver::formats::serialize::To<userver::formats::json::Value>
    /*unused*/
) -> userver::formats::json::Value {
    userver::formats::json::ValueBuilder item;
    item["id"] = boost::uuids::to_string(text_answer.id);
    item["question_id"] = boost::uuids::to_string(text_answer.question_id);
    item["text"] = text_answer.text;
    item["created_at"] =
        userver::utils::datetime::Timestring(text_answer.created_at);
    return item.ExtractValue();
}

auto Parse(
    const userver::formats::json::Value& json,
    userver::formats::parse::To<TextAnswer>
    /*unused*/
) -> TextAnswer {
    return TextAnswer{
        .id = Utils::StringToUuid(json["id"].As<std::string>()),
        .question_id =
            Utils::StringToUuid(json["question_id"].As<std::string>()),
        .text = json["text"].As<std::string>(),
        .created_at = userver::utils::datetime::Stringtime(
            json["created_at"].As<std::string>()
        ),
    };
}

} // namespace Models
