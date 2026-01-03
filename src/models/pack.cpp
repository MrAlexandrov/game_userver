#include "pack.hpp"

#include <boost/uuid/uuid_io.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/serialize/common_containers.hpp>

#include "utils/string_to_uuid.hpp"

namespace Models {

auto Pack::Introspect() const {
    return std::tie(id, data.title);
}

auto Serialize(
    const Pack& pack,
    userver::formats::serialize::To<userver::formats::json::Value>
    /*unused*/
) -> userver::formats::json::Value {
    userver::formats::json::ValueBuilder item;
    item["id"] = boost::uuids::to_string(pack.id);
    item["title"] = pack.data.title;
    return item.ExtractValue();
}

auto Parse(
    const userver::formats::json::Value& json, userver::formats::parse::To<Pack>
    /*unused*/
) -> Pack {
    return Pack{
        .id = Utils::StringToUuid(json["id"].As<std::string>()),
        .data = {
                 .title = json["title"].As<std::string>(),
                 },
    };
}

} // namespace Models
