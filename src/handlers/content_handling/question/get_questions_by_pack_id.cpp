#include "get_questions_by_pack_id.hpp"

#include <userver/components/component_context.hpp>

#include "components/storage/storage.hpp"
#include "utils/string_to_uuid.hpp"

namespace game_userver {

GetQuestionsByPackId::GetQuestionsByPackId(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context),
      storage_(component_context.FindComponent<components::Storage>()) {}

GetQuestionsByPackId::~GetQuestionsByPackId() = default;

auto GetQuestionsByPackId::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
    /*context*/
) const -> std::string {
    const auto& stringPackId = request.GetPathArg("pack_id");

    const auto questions =
        storage_.GetQuestionsByPackId(Utils::StringToUuid(stringPackId));

    userver::formats::json::ValueBuilder result{
        userver::formats::common::Type::kArray
    };

    for (const auto& question : questions) {
        result.PushBack(question);
    }

    return userver::formats::json::ToPrettyString(result.ExtractValue());
}

} // namespace game_userver
