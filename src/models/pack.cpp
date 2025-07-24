#include "pack.hpp"
#include "utils/string_to_uuid.hpp"

#include <boost/uuid/uuid_io.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/serialize/common_containers.hpp>

namespace NModels {

auto Pack::Introspect() const {
    return std::tie(id, title);
}

userver::formats::json::Value Serialize(
      const Pack& pack
    , userver::formats::serialize::To<userver::formats::json::Value>
) {
    userver::formats::json::ValueBuilder item;
    item["id"]      = boost::uuids::to_string(pack.id);
    item["title"]   = pack.title;
    return item.ExtractValue();
}

Pack Parse(
      const userver::formats::json::Value& json
    , userver::formats::parse::To<Pack>
) {    
    Pack pack;
    pack.id = NUtils::GetUuidByString(json["id"].As<std::string>());
    pack.title = json["title"].As<std::string>();    
    return pack;
}

} // namespace NModels
