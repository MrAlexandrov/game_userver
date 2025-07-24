#include "variant.hpp"

#include <boost/uuid/uuid_io.hpp>
#include <userver/formats/json/value_builder.hpp>

namespace NModels {

auto Variant::Introspect() const {
    return std::tie(id, pack_id, text, image_url);
}

userver::formats::json::Value Serialize(
      const Variant& variant
    , userver::formats::serialize::To<userver::formats::json::Value>
) {
    userver::formats::json::ValueBuilder item;
    item["id"]          = boost::uuids::to_string(variant.id);
    item["pack_id"]     = boost::uuids::to_string(variant.pack_id);
    item["text"]        = variant.text;
    item["image_url"]   = variant.image_url;
    return item.ExtractValue();
}

} // namespace NModels
