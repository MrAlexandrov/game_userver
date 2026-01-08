#pragma once

#include "../game_observer.hpp"
#include <userver/logging/log.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace game_userver::logic::game::observers {

// Observer для логирования всех игровых событий
class LoggingObserver : public IGameObserver {
public:
    void OnEvent(const GameEvent& event) override {
        std::visit([this](const auto& e) { LogEvent(e); }, event);
    }

private:
    void LogEvent(const GameSessionCreatedEvent& event) {
        LOG_INFO() << "Game session created: "
                   << boost::uuids::to_string(event.game_session_id)
                   << ", pack_id: " << boost::uuids::to_string(event.pack_id);
    }

    void LogEvent(const PlayerAddedEvent& event) {
        LOG_INFO() << "Player added: " << event.player.name
                   << " (id: " << boost::uuids::to_string(event.player.id)
                   << ") to game "
                   << boost::uuids::to_string(event.game_session_id);
    }

    void LogEvent(const GameStartedEvent& event) {
        LOG_INFO() << "Game started: "
                   << boost::uuids::to_string(event.game_session_id)
                   << ", players: " << event.total_players
                   << ", questions: " << event.total_questions;
    }

    void LogEvent(const QuestionPresentedEvent& event) {
        LOG_INFO() << "Question presented: " << event.question.text
                   << " (index: " << event.question_index << "/"
                   << event.total_questions << ") in game "
                   << boost::uuids::to_string(event.game_session_id);
    }

    void LogEvent(const AnswerSubmittedEvent& event) {
        LOG_INFO() << "Answer submitted by " << event.player_name
                   << " (player_id: "
                   << boost::uuids::to_string(event.player_id)
                   << "): " << (event.is_correct ? "CORRECT" : "INCORRECT")
                   << " in game "
                   << boost::uuids::to_string(event.game_session_id);
    }

    void LogEvent(const AllPlayersAnsweredEvent& event) {
        LOG_INFO() << "All players answered question "
                   << boost::uuids::to_string(event.question_id)
                   << " (index: " << event.question_index
                   << ") in game "
                   << boost::uuids::to_string(event.game_session_id)
                   << ", total answers: " << event.answers_count;
    }

    void LogEvent(const QuestionAdvancedEvent& event) {
        LOG_INFO() << "Question advanced from " << event.previous_question_index
                   << " to " << event.new_question_index << " in game "
                   << boost::uuids::to_string(event.game_session_id);
    }

    void LogEvent(const GameFinishedEvent& event) {
        LOG_INFO() << "Game finished: "
                   << boost::uuids::to_string(event.game_session_id)
                   << ", questions: " << event.total_questions
                   << ", players: " << event.total_players;
    }

    void LogEvent(const PlayerScoreUpdatedEvent& event) {
        LOG_INFO() << "Player score updated: " << event.player_name
                   << " (id: " << boost::uuids::to_string(event.player_id)
                   << ") from " << event.old_score << " to " << event.new_score
                   << " in game "
                   << boost::uuids::to_string(event.game_session_id);
    }
};

} // namespace game_userver::logic::game::observers
