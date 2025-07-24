#include "question.hpp"

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
    item["id"]          = boost::uuids::to_string(question.id);
    item["pack_id"]     = boost::uuids::to_string(question.pack_id);
    item["text"]        = question.text;
    item["image_url"]   = question.image_url;
    return item.ExtractValue();
}

} // namespace NModels
