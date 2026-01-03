#include "variant.hpp"

#include <boost/uuid/uuid_io.hpp>
#include <userver/formats/json/value_builder.hpp>

#include "utils/string_to_uuid.hpp"

namespace Models {

auto Variant::Introspect() const {
    return std::tie(id, question_id, text, is_correct);
}

auto Serialize(
    const Variant& variant,
    userver::formats::serialize::To<userver::formats::json::Value>
    /*unused*/
) -> userver::formats::json::Value {
    userver::formats::json::ValueBuilder item;
    item["id"] = boost::uuids::to_string(variant.id);
    item["question_id"] = boost::uuids::to_string(variant.question_id);
    item["text"] = variant.text;
    item["is_correct"] = variant.is_correct;
    return item.ExtractValue();
}

auto Parse(
    const userver::formats::json::Value& json,
    userver::formats::parse::To<Variant>
    /*unused*/
) -> Variant {
    Variant variant;
    variant.id = Utils::StringToUuid(json["id"].As<std::string>());
    variant.question_id =
        Utils::StringToUuid(json["question_id"].As<std::string>());
    variant.text = json["text"].As<std::string>();
    variant.is_correct =
        Utils::StringToBool(json["is_correct"].As<std::string>());
    return variant;
}

} // namespace Models
