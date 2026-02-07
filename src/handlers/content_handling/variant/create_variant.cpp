#include "create_variant.hpp"

#include <stdexcept>
#include <userver/components/component_context.hpp>

#include "components/storage/storage.hpp"
#include "models/variant.hpp"
#include "utils/string_to_uuid.hpp"
#include "utils/variant.hpp"

namespace game_userver {

CreateVariant::CreateVariant(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context),
      storage_(component_context.FindComponent<components::Storage>()) {}

CreateVariant::~CreateVariant() = default;

auto CreateVariant::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
    /*context*/
) const -> std::string {
    auto variant = Utils::GetVariantFromRequest(request);
    const auto& question_id_str = request.GetPathArg("question_id");
    variant.question_id = Utils::StringToUuid(question_id_str);

    const auto createdVariantOpt = storage_.CreateVariant(variant);

    if (!createdVariantOpt) {
        request.GetHttpResponse().SetStatus(
            userver::server::http::HttpStatus::kInternalServerError
        );
        throw std::runtime_error("Failed to create variant");
    }

    return userver::formats::json::ToPrettyString(
        userver::formats::json::ValueBuilder{createdVariantOpt.value()}
            .ExtractValue()
    );
}

} // namespace game_userver
