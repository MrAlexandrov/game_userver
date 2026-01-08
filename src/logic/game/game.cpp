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

GameService::GameService(ClusterPtr pg_cluster)
    : pg_cluster_(pg_cluster), observer_manager_() {}

void GameService::AddObserver(std::shared_ptr<IGameObserver> observer) {
    observer_manager_.AddObserver(observer);
}

void GameService::RemoveObserver(std::shared_ptr<IGameObserver> observer) {
    observer_manager_.RemoveObserver(observer);
}

void GameService::ClearObservers() { observer_manager_.ClearObservers(); }

GameObserverManager& GameService::GetObserverManager() {
    return observer_manager_;
}

void GameService::NotifyObservers(const GameEvent& event) {
    observer_manager_.NotifyObservers(event);
}

auto GameService::CreateGameSession(const boost::uuids::uuid& pack_id)
    -> std::optional<Models::GameSession> {
    auto game_session = NStorage::CreateGameSession(pg_cluster_, pack_id);
    
    if (game_session) {
        // Уведомляем наблюдателей о создании игровой сессии
        NotifyObservers(GameSessionCreatedEvent(game_session->id, pack_id));
    }
    
    return game_session;
}

auto GameService::AddPlayer(
    const boost::uuids::uuid& game_session_id, const std::string& player_name
) -> std::optional<Models::Player> {
    auto player = NStorage::AddPlayer(pg_cluster_, game_session_id, player_name);
    
    if (player) {
        // Уведомляем наблюдателей о добавлении игрока
        NotifyObservers(PlayerAddedEvent(game_session_id, *player));
    }
    
    return player;
}

auto GameService::StartGame(const boost::uuids::uuid& game_session_id)
    -> std::optional<Models::GameSession> {
    auto game_session = NStorage::StartGameSession(pg_cluster_, game_session_id);
    
    if (game_session) {
        // Получаем количество игроков и вопросов
        auto players = NStorage::GetPlayersByGameSessionId(pg_cluster_, game_session_id);
        auto questions_and_variants = NStorage::GetQuestionsAndVariantsByPackId(
            pg_cluster_, game_session->pack_id
        );
        
        // Уведомляем наблюдателей о старте игры
        NotifyObservers(GameStartedEvent(
            game_session_id,
            static_cast<int>(players.size()),
            static_cast<int>(questions_and_variants.size())
        ));
        
        // Уведомляем о представлении первого вопроса
        if (!questions_and_variants.empty()) {
            const auto& [question, _] = questions_and_variants[0];
            NotifyObservers(QuestionPresentedEvent(
                game_session_id, question, 0,
                static_cast<int>(questions_and_variants.size())
            ));
        }
    }
    
    return game_session;
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

    // Уведомляем наблюдателей об отправке ответа
    NotifyObservers(AnswerSubmittedEvent(
        game_session->id, player_id, current_question.id, variant_id,
        is_correct, player->name
    ));

    // If the answer is correct, update the player's score
    int old_score = player->score;
    if (is_correct) {
        auto updated_player =
            NStorage::UpdatePlayerScore(pg_cluster_, player_id, 1);
        if (updated_player) {
            // Уведомляем наблюдателей об обновлении счёта
            NotifyObservers(PlayerScoreUpdatedEvent(
                game_session->id, player_id, player->name, old_score,
                updated_player->score
            ));
        }
    }

    // Get all players in this game session
    auto all_players =
        NStorage::GetPlayersByGameSessionId(pg_cluster_, game_session->id);

    // Get count of answers for current question
    auto answers_count = NStorage::GetAnswersCountForQuestion(
        pg_cluster_, game_session->id, current_question.id
    );

    // Check if all players have answered
    bool all_players_answered =
        (answers_count >= static_cast<int>(all_players.size()));

    // Check if this was the last question
    bool is_last_question =
        (game_session->current_question_index ==
         static_cast<int>(questions_and_variants.size()) - 1);

    if (all_players_answered) {
        // Уведомляем наблюдателей о том, что все игроки ответили
        NotifyObservers(AllPlayersAnsweredEvent(
            game_session->id, current_question.id,
            game_session->current_question_index,
            static_cast<int>(all_players.size()), answers_count
        ));
    }

    if (is_last_question && all_players_answered) {
        // All players answered the last question - end the game
        NStorage::EndGameSession(pg_cluster_, player->game_session_id);
        
        // Уведомляем наблюдателей о завершении игры
        NotifyObservers(GameFinishedEvent(
            game_session->id,
            static_cast<int>(questions_and_variants.size()),
            static_cast<int>(all_players.size())
        ));
        
        return GameResult::kGameFinished;
    } else if (!is_last_question && all_players_answered) {
        // All players answered - advance to the next question
        int new_index = game_session->current_question_index + 1;
        NStorage::AdvanceToNextQuestion(
            pg_cluster_, player->game_session_id, new_index
        );
        
        // Уведомляем наблюдателей о переходе к следующему вопросу
        NotifyObservers(QuestionAdvancedEvent(
            game_session->id, game_session->current_question_index, new_index
        ));
        
        // Уведомляем о представлении нового вопроса
        if (new_index < static_cast<int>(questions_and_variants.size())) {
            const auto& [next_question, _] = questions_and_variants[new_index];
            NotifyObservers(QuestionPresentedEvent(
                game_session->id, next_question, new_index,
                static_cast<int>(questions_and_variants.size())
            ));
        }
    }
    // If not all players answered yet, just return the result without advancing

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
