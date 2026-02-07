#include "get_variant_by_id.hpp"

#include <userver/components/component_context.hpp>

#include "components/storage/storage.hpp"
#include "utils/string_to_uuid.hpp"

namespace game_userver {

GetVariantById::GetVariantById(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context),
      storage_(component_context.FindComponent<components::Storage>()) {}

GetVariantById::~GetVariantById() = default;

auto GetVariantById::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
    /*context*/
) const -> std::string {
    const auto& stringId = request.GetPathArg("variant_id");

    const auto id = Utils::StringToUuid(stringId);
    if (id.is_nil()) {
        return "Incorrect id";
    }

    const auto variantOpt = storage_.GetVariantById(id);
    if (!variantOpt) {
        return {};
    }

    return userver::formats::json::ToPrettyString(
        userver::formats::json::ValueBuilder{variantOpt.value()}.ExtractValue()
    );
}

} // namespace game_userver
