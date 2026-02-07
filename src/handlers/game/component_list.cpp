#include "component_list.hpp"

#include "add_player.hpp"
#include "create_game_session.hpp"
#include "get_game_results.hpp"
#include "get_game_state.hpp"
#include "start_game.hpp"
#include "submit_answer.hpp"

namespace game_userver {

auto GetGameHandlers() -> userver::components::ComponentList {
    return userver::components::ComponentList()
        .Append<CreateGameSession>()
        .Append<AddPlayer>()
        .Append<StartGame>()
        .Append<GetGameState>()
        .Append<SubmitAnswer>()
        .Append<GetGameResults>();
}

} // namespace game_userver
