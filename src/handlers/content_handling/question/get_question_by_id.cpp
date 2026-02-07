#include "get_question_by_id.hpp"

#include <userver/components/component_context.hpp>

#include "components/storage/storage.hpp"
#include "utils/string_to_uuid.hpp"

namespace game_userver {

GetQuestionById::GetQuestionById(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context),
      storage_(component_context.FindComponent<components::Storage>()) {}

GetQuestionById::~GetQuestionById() = default;

auto GetQuestionById::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
    /*context*/
) const -> std::string {
    const auto& stringId = request.GetPathArg("question_id");

    const auto id = Utils::StringToUuid(stringId);
    if (id.is_nil()) {
        return "Incorrect id";
    }

    const auto questionOpt = storage_.GetQuestionById(id);
    if (!questionOpt) {
        return {};
    }

    return userver::formats::json::ToPrettyString(
        userver::formats::json::ValueBuilder{questionOpt.value()}.ExtractValue()
    );
}

} // namespace game_userver
