#include "game.hpp"

#include <userver/utils/assert.hpp>

#include "storage/game_sessions.hpp"
#include "storage/player_answers.hpp"
#include "storage/players.hpp"
#include "storage/questions.hpp"
#include "storage/questions_and_variants.hpp"
#include "storage/variants.hpp"

#include <fmt/format.h>

namespace game_userver::logic::game {

GameService::GameService(ClusterPtr pg_cluster) : pg_cluster_(pg_cluster) {}

auto GameService::CreateGameSession(const boost::uuids::uuid& pack_id)
    -> std::optional<Models::GameSession> {
    return NStorage::CreateGameSession(pg_cluster_, pack_id);
}

auto GameService::AddPlayer(
    const boost::uuids::uuid& game_session_id, const std::string& player_name
) -> std::optional<Models::Player> {
    return NStorage::AddPlayer(pg_cluster_, game_session_id, player_name);
}

auto GameService::StartGame(const boost::uuids::uuid& game_session_id)
    -> std::optional<Models::GameSession> {
    return NStorage::StartGameSession(pg_cluster_, game_session_id);
}

auto GameService::GetCurrentQuestion(const boost::uuids::uuid& game_session_id)
    -> std::optional<GameQuestion> {
    // Get the game session to know which pack and current question index
    auto game_session =
        NStorage::GetGameSessionById(pg_cluster_, game_session_id);
    if (!game_session) {
        return std::nullopt;
    }

    // Get all questions and variants for this pack
    auto questions_and_variants = NStorage::GetQuestionsAndVariantsByPackId(
        pg_cluster_, game_session->pack_id
    );

    // Check if we have questions
    if (questions_and_variants.empty()) {
        return std::nullopt;
    }

    // Check if we're within bounds
    if (game_session->current_question_index >=
        static_cast<int>(questions_and_variants.size())) {
        return std::nullopt;
    }

    // Get the current question and its variants
    const auto& [question, variants] =
        questions_and_variants[game_session->current_question_index];

    GameQuestion game_question;
    game_question.question = question;
    game_question.variants = variants;

    return game_question;
}

auto GameService::SubmitAnswer(
    const boost::uuids::uuid& player_id, const boost::uuids::uuid& variant_id
) -> GameResult {
    // First, check if the variant is correct
    auto is_correct_opt =
        NStorage::CheckVariantCorrectnessById(pg_cluster_, variant_id);
    if (!is_correct_opt) {
        return GameResult::kError;
    }

    bool is_correct = is_correct_opt.value();

    // Get the player to know which game session they're in
    auto player = NStorage::GetPlayerById(pg_cluster_, player_id);
    if (!player) {
        return GameResult::kError;
    }

    // Get the game session to know which question was answered
    auto game_session =
        NStorage::GetGameSessionById(pg_cluster_, player->game_session_id);
    if (!game_session) {
        return GameResult::kError;
    }

    // Get the current question
    auto questions_and_variants = NStorage::GetQuestionsAndVariantsByPackId(
        pg_cluster_, game_session->pack_id
    );

    if (questions_and_variants.empty()) {
        return GameResult::kError;
    }

    if (game_session->current_question_index >=
        static_cast<int>(questions_and_variants.size())) {
        return GameResult::kError;
    }

    const auto& [current_question, _] =
        questions_and_variants[game_session->current_question_index];

    // Submit the player's answer
    auto player_answer = NStorage::SubmitPlayerAnswer(
        pg_cluster_, player_id, current_question.id, variant_id, is_correct
    );

    if (!player_answer) {
        return GameResult::kError;
    }

    // If the answer is correct, update the player's score
    if (is_correct) {
        auto updated_player =
            NStorage::UpdatePlayerScore(pg_cluster_, player_id, 1);
        if (!updated_player) {
            // Not a critical error, we can still return kCorrect
        }
    }

    // Check if this was the last question
    if (game_session->current_question_index ==
        static_cast<int>(questions_and_variants.size()) - 1) {
        // End the game
        NStorage::EndGameSession(pg_cluster_, player->game_session_id);
        return is_correct ? GameResult::kGameFinished
                          : GameResult::kGameFinished;
    } else {
        // Advance to the next question
        NStorage::AdvanceToNextQuestion(
            pg_cluster_, player->game_session_id,
            game_session->current_question_index + 1
        );
    }

    return is_correct ? GameResult::kCorrect : GameResult::kIncorrect;
}

auto GameService::GetGameSession(const boost::uuids::uuid& game_session_id)
    -> std::optional<Models::GameSession> {
    return NStorage::GetGameSessionById(pg_cluster_, game_session_id);
}

auto GameService::GetPlayers(const boost::uuids::uuid& game_session_id)
    -> std::vector<Models::Player> {
    return NStorage::GetPlayersByGameSessionId(pg_cluster_, game_session_id);
}

auto GameService::GetPlayerAnswers(const boost::uuids::uuid& player_id)
    -> std::vector<Models::PlayerAnswer> {
    return NStorage::GetPlayerAnswersByPlayerId(pg_cluster_, player_id);
}

} // namespace game_userver::logic::game
