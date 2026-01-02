#include "get_all_packs.hpp"

#include <userver/components/component_context.hpp>
#include <userver/logging/log.hpp>

#include "services/pack_service.hpp"

namespace game_userver {

struct GetAllPacks::Impl {
    services::PackService& pack_service;

    explicit Impl(const userver::components::ComponentContext& context)
        : pack_service(context.FindComponent<services::PackService>()) {}
};

GetAllPacks::GetAllPacks(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context), impl_(component_context) {}

GetAllPacks::~GetAllPacks() = default;

std::string GetAllPacks::HandleRequestThrow(
    const userver::server::http::HttpRequest&,
    userver::server::request::RequestContext&
) const {
    const auto packs = impl_->pack_service.GetAllPacks();

    userver::formats::json::ValueBuilder result{
        userver::formats::common::Type::kArray
    };

    for (const auto& pack : packs) {
        result.PushBack(pack);
    }

    return userver::formats::json::ToPrettyString(result.ExtractValue());
}

} // namespace game_userver
