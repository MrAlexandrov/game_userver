#include "component_list.hpp"

#include "add_player.hpp"
#include "create_game_session.hpp"
#include "get_current_question.hpp"
#include "get_game_results.hpp"
#include "start_game.hpp"
#include "submit_answer.hpp"

namespace game_userver {

void AppendGameHandlers(userver::components::ComponentList& component_list) {
    component_list.Append<CreateGameSession>();
    component_list.Append<AddPlayer>();
    component_list.Append<StartGame>();
    component_list.Append<GetCurrentQuestion>();
    component_list.Append<SubmitAnswer>();
    component_list.Append<GetGameResults>();
}

} // namespace game_userver
