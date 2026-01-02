#include "get_pack_by_id.hpp"

#include <userver/components/component_context.hpp>

#include "services/pack_service.hpp"
#include "utils/string_to_uuid.hpp"

namespace game_userver {

struct GetPack::Impl {
    services::PackService& pack_service;

    explicit Impl(const userver::components::ComponentContext& context)
        : pack_service(context.FindComponent<services::PackService>()) {}
};

GetPack::GetPack(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context), impl_(component_context) {}

GetPack::~GetPack() = default;

std::string GetPack::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    const auto& stringUuid = request.GetArg("uuid");

    const auto uuid = Utils::StringToUuid(stringUuid);
    if (uuid.is_nil()) {
        return "Incorrect uuid";
    }

    const auto packOpt = impl_->pack_service.GetPackById(uuid);
    if (!packOpt) {
        return {};
    }

    return userver::formats::json::ToPrettyString(
        userver::formats::json::ValueBuilder{packOpt.value()}.ExtractValue()
    );
}

} // namespace game_userver
