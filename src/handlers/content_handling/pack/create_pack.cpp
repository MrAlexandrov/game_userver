#include "create_pack.hpp"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <userver/components/component_context.hpp>
#include <userver/logging/log.hpp>

#include "services/pack_service.hpp"

namespace game_userver {

struct CreatePack::Impl {
    services::PackService& pack_service;

    explicit Impl(const userver::components::ComponentContext& context)
        : pack_service(context.FindComponent<services::PackService>()) {}
};

CreatePack::CreatePack(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context), impl_(component_context) {}

CreatePack::~CreatePack() = default;

std::string CreatePack::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    using userver::logging::Level::kDebug;

    const auto& title = request.GetArg("title");
    LOG(kDebug) << "title: " << title;

    const auto createdPackOpt = impl_->pack_service.CreatePack(title);
    if (!createdPackOpt) {
        request.GetHttpResponse().SetStatus(
            userver::server::http::HttpStatus::kInternalServerError
        );
        return {};
    }
    const auto& [id, pack_title] = createdPackOpt.value();

    LOG(kDebug) << "inserted pack:\n"
                << boost::uuids::to_string(id) << " " << pack_title;

    return userver::formats::json::ToPrettyString(
        userver::formats::json::ValueBuilder{createdPackOpt.value()}
            .ExtractValue()
    );
}

} // namespace game_userver
