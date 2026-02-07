#include "get_variants_by_question_id.hpp"

#include <userver/components/component_context.hpp>

#include "components/storage/storage.hpp"
#include "utils/string_to_uuid.hpp"

namespace game_userver {

GetVariantsByQuestionId::GetVariantsByQuestionId(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context),
      storage_(component_context.FindComponent<components::Storage>()) {}

GetVariantsByQuestionId::~GetVariantsByQuestionId() = default;

auto GetVariantsByQuestionId::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
    /*context*/
) const -> std::string {
    const auto& stringQuestionId = request.GetPathArg("question_id");

    const auto variants =
        storage_.GetVariantsByQuestionId(Utils::StringToUuid(stringQuestionId));

    userver::formats::json::ValueBuilder result{
        userver::formats::common::Type::kArray
    };

    for (const auto& variant : variants) {
        result.PushBack(variant);
    }

    return userver::formats::json::ToPrettyString(result.ExtractValue());
}

} // namespace game_userver
