#pragma once

#include <chrono>
#include <string>
#include <variant>

#include "models/game_session.hpp"
#include "models/pack.hpp"
#include "models/player.hpp"
#include "models/question.hpp"
#include "models/variant.hpp"

namespace game_userver::logic::game {

// Типы игровых событий
enum class GameEventType {
    kGameSessionCreated,
    kPlayerAdded,
    kGameStarted,
    kQuestionPresented,
    kAnswerSubmitted,
    kAllPlayersAnswered,
    kQuestionAdvanced,
    kGameFinished,
    kPlayerScoreUpdated
};

// Базовая структура события
struct GameEventBase {
    GameEventType type;
    std::chrono::system_clock::time_point timestamp;
    Models::GameSession::GameSessionId game_session_id;

    GameEventBase(
        GameEventType event_type,
        const Models::GameSession::GameSessionId& session_id
    )
        : type(event_type), timestamp(std::chrono::system_clock::now()),
          game_session_id(session_id) {}
};

// Событие: Создана игровая сессия
struct GameSessionCreatedEvent : GameEventBase {
    Models::Pack::PackId pack_id;

    GameSessionCreatedEvent(
        const Models::GameSession::GameSessionId& session_id,
        const Models::Pack::PackId& pack_id_val
    )
        : GameEventBase(GameEventType::kGameSessionCreated, session_id),
          pack_id(pack_id_val) {}
};

// Событие: Добавлен игрок
struct PlayerAddedEvent : GameEventBase {
    Models::Player player;

    PlayerAddedEvent(
        const Models::GameSession::GameSessionId& session_id,
        const Models::Player& player_val
    )
        : GameEventBase(GameEventType::kPlayerAdded, session_id),
          player(player_val) {}
};

// Событие: Игра началась
struct GameStartedEvent : GameEventBase {
    int total_players;
    int total_questions;

    GameStartedEvent(
        const Models::GameSession::GameSessionId& session_id, int players,
        int questions
    )
        : GameEventBase(GameEventType::kGameStarted, session_id),
          total_players(players), total_questions(questions) {}
};

// Событие: Представлен вопрос
struct QuestionPresentedEvent : GameEventBase {
    Models::Question question;
    int question_index;
    int total_questions;

    QuestionPresentedEvent(
        const Models::GameSession::GameSessionId& session_id,
        const Models::Question& question_val, int index, int total
    )
        : GameEventBase(GameEventType::kQuestionPresented, session_id),
          question(question_val), question_index(index),
          total_questions(total) {}
};

// Событие: Отправлен ответ
struct AnswerSubmittedEvent : GameEventBase {
    Models::Player::PlayerId player_id;
    Models::Question::QuestionId question_id;
    Models::Variant::VariantId variant_id;
    bool is_correct;
    std::string player_name;

    AnswerSubmittedEvent(
        const Models::GameSession::GameSessionId& session_id,
        const Models::Player::PlayerId& player_id_val,
        const Models::Question::QuestionId& question_id_val,
        const Models::Variant::VariantId& variant_id_val, bool correct,
        const std::string& name
    )
        : GameEventBase(GameEventType::kAnswerSubmitted, session_id),
          player_id(player_id_val), question_id(question_id_val),
          variant_id(variant_id_val), is_correct(correct), player_name(name) {}
};

// Событие: Все игроки ответили на вопрос
struct AllPlayersAnsweredEvent : GameEventBase {
    Models::Question::QuestionId question_id;
    int question_index;
    int total_players;
    int answers_count;

    AllPlayersAnsweredEvent(
        const Models::GameSession::GameSessionId& session_id,
        const Models::Question::QuestionId& question_id_val, int index,
        int players, int answers
    )
        : GameEventBase(GameEventType::kAllPlayersAnswered, session_id),
          question_id(question_id_val), question_index(index),
          total_players(players), answers_count(answers) {}
};

// Событие: Переход к следующему вопросу
struct QuestionAdvancedEvent : GameEventBase {
    int previous_question_index;
    int new_question_index;

    QuestionAdvancedEvent(
        const Models::GameSession::GameSessionId& session_id, int prev_index,
        int new_index
    )
        : GameEventBase(GameEventType::kQuestionAdvanced, session_id),
          previous_question_index(prev_index), new_question_index(new_index) {}
};

// Событие: Игра завершена
struct GameFinishedEvent : GameEventBase {
    int total_questions;
    int total_players;

    GameFinishedEvent(
        const Models::GameSession::GameSessionId& session_id, int questions,
        int players
    )
        : GameEventBase(GameEventType::kGameFinished, session_id),
          total_questions(questions), total_players(players) {}
};

// Событие: Обновлён счёт игрока
struct PlayerScoreUpdatedEvent : GameEventBase {
    Models::Player::PlayerId player_id;
    std::string player_name;
    int old_score;
    int new_score;

    PlayerScoreUpdatedEvent(
        const Models::GameSession::GameSessionId& session_id,
        const Models::Player::PlayerId& player_id_val, const std::string& name,
        int old_val, int new_val
    )
        : GameEventBase(GameEventType::kPlayerScoreUpdated, session_id),
          player_id(player_id_val), player_name(name), old_score(old_val),
          new_score(new_val) {}
};

// Вариант для всех типов событий
using GameEvent = std::variant<
    GameSessionCreatedEvent, PlayerAddedEvent, GameStartedEvent,
    QuestionPresentedEvent, AnswerSubmittedEvent, AllPlayersAnsweredEvent,
    QuestionAdvancedEvent, GameFinishedEvent, PlayerScoreUpdatedEvent>;

} // namespace game_userver::logic::game
