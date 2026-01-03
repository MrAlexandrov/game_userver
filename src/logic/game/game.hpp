#pragma once

#include <boost/uuid/uuid.hpp>
#include <optional>
#include <string>
#include <vector>

#include <userver/storages/postgres/cluster.hpp>

#include "models/game_session.hpp"
#include "models/player.hpp"
#include "models/player_answer.hpp"
#include "models/question.hpp"
#include "models/variant.hpp"

namespace game_userver::logic::game {

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

    [[nodiscard]] auto CreateGameSession(const boost::uuids::uuid& pack_id)
        -> std::optional<Models::GameSession>;

    [[nodiscard]] auto AddPlayer(
        const boost::uuids::uuid& game_session_id,
        const std::string& player_name
    ) -> std::optional<Models::Player>;

    [[nodiscard]] auto StartGame(const boost::uuids::uuid& game_session_id)
        -> std::optional<Models::GameSession>;

    [[nodiscard]] auto
    GetCurrentQuestion(const boost::uuids::uuid& game_session_id)
        -> std::optional<GameQuestion>;

    [[nodiscard]] auto SubmitAnswer(
        const boost::uuids::uuid& player_id,
        const boost::uuids::uuid& variant_id
    ) -> GameResult;

    [[nodiscard]] auto GetGameSession(const boost::uuids::uuid& game_session_id)
        -> std::optional<Models::GameSession>;

    [[nodiscard]] auto GetPlayers(const boost::uuids::uuid& game_session_id)
        -> std::vector<Models::Player>;

    [[nodiscard]] auto GetPlayerAnswers(const boost::uuids::uuid& player_id)
        -> std::vector<Models::PlayerAnswer>;

private:
    ClusterPtr pg_cluster_;
};

} // namespace game_userver::logic::game
