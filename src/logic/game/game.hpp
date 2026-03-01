#pragma once

#include <optional>
#include <string>
#include <vector>

#include <userver/storages/postgres/cluster.hpp>

#include "game_observer.hpp"
#include "logic/validators/answer_validator.hpp"
#include "models/game_session.hpp"
#include "models/pack.hpp"
#include "models/player.hpp"
#include "models/player_answer.hpp"
#include "models/question.hpp"
#include "models/variant.hpp"

namespace game_userver::logic::game {

using game_userver::logic::validators::PlayerAnswerInput;
using userver::storages::postgres::ClusterPtr;

enum class GameResult {
    kCorrect,
    kIncorrect,
    kGameFinished,
    kError
};

struct GameQuestion {
    Models::Question question;
    std::vector<Models::Variant> variants;
};

class GameService final {
public:
    explicit GameService(ClusterPtr pg_cluster);

    [[nodiscard]] auto CreateGameSession(const Models::Pack::PackId& pack_id)
        -> std::optional<Models::GameSession>;

    [[nodiscard]] auto AddPlayer(
        const Models::GameSession::GameSessionId& game_session_id,
        const std::string& player_name
    ) -> std::optional<Models::Player>;

    [[nodiscard]] auto
    StartGame(const Models::GameSession::GameSessionId& game_session_id)
        -> std::optional<Models::GameSession>;

    [[nodiscard]] auto GetCurrentQuestion(
        const Models::GameSession::GameSessionId& game_session_id
    ) -> std::optional<GameQuestion>;

    [[nodiscard]] auto SubmitAnswer(
        const Models::Player::PlayerId& player_id,
        const PlayerAnswerInput& answer_input
    ) -> GameResult;

    [[nodiscard]] auto
    GetGameSession(const Models::GameSession::GameSessionId& game_session_id)
        -> std::optional<Models::GameSession>;

    [[nodiscard]] auto
    GetPlayers(const Models::GameSession::GameSessionId& game_session_id)
        -> std::vector<Models::Player>;

    [[nodiscard]] auto
    GetPlayerAnswers(const Models::Player::PlayerId& player_id)
        -> std::vector<Models::PlayerAnswer>;

    // Observer management
    void AddObserver(std::shared_ptr<IGameObserver> observer);
    void RemoveObserver(std::shared_ptr<IGameObserver> observer);
    void ClearObservers();
    [[nodiscard]] GameObserverManager& GetObserverManager();

private:
    void NotifyObservers(const GameEvent& event);

    ClusterPtr pg_cluster_;
    GameObserverManager observer_manager_;
};

} // namespace game_userver::logic::game
