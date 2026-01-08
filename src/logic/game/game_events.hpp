#pragma once

#include <boost/uuid/uuid.hpp>
#include <chrono>
#include <string>
#include <variant>

#include "models/game_session.hpp"
#include "models/player.hpp"
#include "models/player_answer.hpp"
#include "models/question.hpp"

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
    boost::uuids::uuid game_session_id;

    GameEventBase(
        GameEventType event_type, const boost::uuids::uuid& session_id
    )
        : type(event_type),
          timestamp(std::chrono::system_clock::now()),
          game_session_id(session_id) {}
};

// Событие: Создана игровая сессия
struct GameSessionCreatedEvent : GameEventBase {
    boost::uuids::uuid pack_id;

    GameSessionCreatedEvent(
        const boost::uuids::uuid& session_id,
        const boost::uuids::uuid& pack_id_val
    )
        : GameEventBase(GameEventType::kGameSessionCreated, session_id),
          pack_id(pack_id_val) {}
};

// Событие: Добавлен игрок
struct PlayerAddedEvent : GameEventBase {
    Models::Player player;

    PlayerAddedEvent(
        const boost::uuids::uuid& session_id, const Models::Player& player_val
    )
        : GameEventBase(GameEventType::kPlayerAdded, session_id),
          player(player_val) {}
};

// Событие: Игра началась
struct GameStartedEvent : GameEventBase {
    int total_players;
    int total_questions;

    GameStartedEvent(
        const boost::uuids::uuid& session_id, int players, int questions
    )
        : GameEventBase(GameEventType::kGameStarted, session_id),
          total_players(players),
          total_questions(questions) {}
};

// Событие: Представлен вопрос
struct QuestionPresentedEvent : GameEventBase {
    Models::Question question;
    int question_index;
    int total_questions;

    QuestionPresentedEvent(
        const boost::uuids::uuid& session_id,
        const Models::Question& question_val, int index, int total
    )
        : GameEventBase(GameEventType::kQuestionPresented, session_id),
          question(question_val),
          question_index(index),
          total_questions(total) {}
};

// Событие: Отправлен ответ
struct AnswerSubmittedEvent : GameEventBase {
    boost::uuids::uuid player_id;
    boost::uuids::uuid question_id;
    boost::uuids::uuid variant_id;
    bool is_correct;
    std::string player_name;

    AnswerSubmittedEvent(
        const boost::uuids::uuid& session_id,
        const boost::uuids::uuid& player_id_val,
        const boost::uuids::uuid& question_id_val,
        const boost::uuids::uuid& variant_id_val, bool correct,
        const std::string& name
    )
        : GameEventBase(GameEventType::kAnswerSubmitted, session_id),
          player_id(player_id_val),
          question_id(question_id_val),
          variant_id(variant_id_val),
          is_correct(correct),
          player_name(name) {}
};

// Событие: Все игроки ответили на вопрос
struct AllPlayersAnsweredEvent : GameEventBase {
    boost::uuids::uuid question_id;
    int question_index;
    int total_players;
    int answers_count;

    AllPlayersAnsweredEvent(
        const boost::uuids::uuid& session_id,
        const boost::uuids::uuid& question_id_val, int index, int players,
        int answers
    )
        : GameEventBase(GameEventType::kAllPlayersAnswered, session_id),
          question_id(question_id_val),
          question_index(index),
          total_players(players),
          answers_count(answers) {}
};

// Событие: Переход к следующему вопросу
struct QuestionAdvancedEvent : GameEventBase {
    int previous_question_index;
    int new_question_index;

    QuestionAdvancedEvent(
        const boost::uuids::uuid& session_id, int prev_index, int new_index
    )
        : GameEventBase(GameEventType::kQuestionAdvanced, session_id),
          previous_question_index(prev_index),
          new_question_index(new_index) {}
};

// Событие: Игра завершена
struct GameFinishedEvent : GameEventBase {
    int total_questions;
    int total_players;

    GameFinishedEvent(
        const boost::uuids::uuid& session_id, int questions, int players
    )
        : GameEventBase(GameEventType::kGameFinished, session_id),
          total_questions(questions),
          total_players(players) {}
};

// Событие: Обновлён счёт игрока
struct PlayerScoreUpdatedEvent : GameEventBase {
    boost::uuids::uuid player_id;
    std::string player_name;
    int old_score;
    int new_score;

    PlayerScoreUpdatedEvent(
        const boost::uuids::uuid& session_id,
        const boost::uuids::uuid& player_id_val, const std::string& name,
        int old_val, int new_val
    )
        : GameEventBase(GameEventType::kPlayerScoreUpdated, session_id),
          player_id(player_id_val),
          player_name(name),
          old_score(old_val),
          new_score(new_val) {}
};

// Вариант для всех типов событий
using GameEvent = std::variant<
    GameSessionCreatedEvent, PlayerAddedEvent, GameStartedEvent,
    QuestionPresentedEvent, AnswerSubmittedEvent, AllPlayersAnsweredEvent,
    QuestionAdvancedEvent, GameFinishedEvent, PlayerScoreUpdatedEvent>;

} // namespace game_userver::logic::game