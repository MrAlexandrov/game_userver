#include "create_question.hpp"

#include <userver/components/component_context.hpp>

#include "components/storage/storage.hpp"
#include "models/question.hpp"
#include "utils/question.hpp"
#include "utils/string_to_uuid.hpp"

namespace game_userver {

CreateQuestion::CreateQuestion(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context),
      storage_(component_context.FindComponent<components::Storage>()) {}

CreateQuestion::~CreateQuestion() = default;

auto CreateQuestion::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
    /*context*/
) const -> std::string {
    auto question = Utils::GetQuestionFromRequest(request);
    const auto& pack_id_str = request.GetPathArg("pack_id");
    question.pack_id = Utils::StringToUuid(pack_id_str);
    const auto createdQuestionOpt =
        storage_.CreateQuestion(std::move(question));

    if (!createdQuestionOpt) {
        request.GetHttpResponse().SetStatus(
            userver::server::http::HttpStatus::kInternalServerError
        );
        throw std::runtime_error("Failed to create question");
    }

    return userver::formats::json::ToPrettyString(
        userver::formats::json::ValueBuilder{createdQuestionOpt.value()}
            .ExtractValue()
    );
}

} // namespace game_userver
