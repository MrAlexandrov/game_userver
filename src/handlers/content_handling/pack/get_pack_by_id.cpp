#include "get_pack_by_id.hpp"

#include <userver/components/component_context.hpp>

#include "components/storage/storage.hpp"
#include "utils/string_to_uuid.hpp"

namespace game_userver {

GetPack::GetPack(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context),
      storage_(component_context.FindComponent<components::Storage>()) {}

GetPack::~GetPack() = default;

auto GetPack::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
    /*context*/
) const -> std::string {
    const auto& stringUuid = request.GetPathArg("pack_id");

    const auto pack_id = Utils::StringToUuid(stringUuid);
    if (pack_id.is_nil()) {
        return "Incorrect uuid";
    }

    const auto packOpt = storage_.GetPackById(pack_id);
    if (!packOpt) {
        return {};
    }

    return userver::formats::json::ToPrettyString(
        userver::formats::json::ValueBuilder{packOpt.value()}.ExtractValue()
    );
}

} // namespace game_userver
