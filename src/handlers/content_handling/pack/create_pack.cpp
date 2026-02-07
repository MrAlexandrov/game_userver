#include "create_pack.hpp"

#include <boost/uuid/uuid.hpp> // NOLINT
#include <boost/uuid/uuid_io.hpp>
#include <userver/components/component_context.hpp>
#include <userver/logging/log.hpp>

#include "components/storage/storage.hpp"
#include "models/pack.hpp"
#include "utils/pack.hpp"

namespace game_userver {

CreatePack::CreatePack(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context),
      storage_(component_context.FindComponent<components::Storage>()) {}

CreatePack::~CreatePack() = default;

auto CreatePack::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
    /*context*/
) const -> std::string {
    using userver::logging::Level::kDebug;

    const auto pack = Utils::GetPackFromRequest(request);

    const auto createdPackOpt = storage_.CreatePack(pack);
    if (!createdPackOpt) {
        request.GetHttpResponse().SetStatus(
            userver::server::http::HttpStatus::kInternalServerError
        );
        throw std::runtime_error("Failed to create pack");
    }
    const auto& [id, title] = createdPackOpt.value();

    LOG(kDebug) << "inserted pack:\n"
                << boost::uuids::to_string(id) << " " << title;

    return userver::formats::json::ToPrettyString(
        userver::formats::json::ValueBuilder{createdPackOpt.value()}
            .ExtractValue()
    );
}

} // namespace game_userver
