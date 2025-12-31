#include "component_list.hpp"

#include "create_question.hpp"
#include "get_question_by_id.hpp"
#include "get_questions_by_pack_id.hpp"

namespace game_userver::question {

userver::components::ComponentList GetQuestionHandlersComponentList() {
    return userver::components::ComponentList()
        .Append<CreateQuestion>()
        .Append<GetQuestionById>()
        .Append<GetQuestionsByPackId>();
}

} // namespace game_userver::question
