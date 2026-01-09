#pragma once

#include "../game_observer.hpp"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <map>
#include <mutex>

namespace game_userver::logic::game::observers {

// Статистика по игровой сессии
struct GameSessionStats {
    boost::uuids::uuid game_session_id;
    int total_players = 0;
    int total_questions = 0;
    int questions_answered = 0;
    int total_answers_submitted = 0;
    int correct_answers = 0;
    int incorrect_answers = 0;
    bool is_finished = false;
    std::chrono::system_clock::time_point started_at;
    std::chrono::system_clock::time_point finished_at;
};

// Observer для сбора статистики
class StatisticsObserver : public IGameObserver {
public:
    void OnEvent(const GameEvent& event) override {
        std::visit(
            [this](const auto& e) {
                ProcessEvent(e);
            },
            event
        );
    }

    // Получить статистику по игровой сессии
    [[nodiscard]] std::optional<GameSessionStats>
    GetStats(const boost::uuids::uuid& game_session_id) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = stats_.find(game_session_id);
        if (it != stats_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    // Получить все статистики
    [[nodiscard]] std::map<boost::uuids::uuid, GameSessionStats>
    GetAllStats() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return stats_;
    }

    // Очистить статистику
    void ClearStats() {
        std::lock_guard<std::mutex> lock(mutex_);
        stats_.clear();
    }

private:
    void ProcessEvent(const GameSessionCreatedEvent& event) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& stats = stats_[event.game_session_id];
        stats.game_session_id = event.game_session_id;
    }

    void ProcessEvent(const PlayerAddedEvent& event) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& stats = stats_[event.game_session_id];
        stats.total_players++;
    }

    void ProcessEvent(const GameStartedEvent& event) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& stats = stats_[event.game_session_id];
        stats.total_questions = event.total_questions;
        stats.started_at = event.timestamp;
    }

    void ProcessEvent(const QuestionPresentedEvent& event) {
        // Можно добавить дополнительную логику если нужно
    }

    void ProcessEvent(const AnswerSubmittedEvent& event) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& stats = stats_[event.game_session_id];
        stats.total_answers_submitted++;
        if (event.is_correct) {
            stats.correct_answers++;
        } else {
            stats.incorrect_answers++;
        }
    }

    void ProcessEvent(const AllPlayersAnsweredEvent& event) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& stats = stats_[event.game_session_id];
        stats.questions_answered++;
    }

    void ProcessEvent(const QuestionAdvancedEvent& event) {
        // Можно добавить дополнительную логику если нужно
    }

    void ProcessEvent(const GameFinishedEvent& event) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& stats = stats_[event.game_session_id];
        stats.is_finished = true;
        stats.finished_at = event.timestamp;
    }

    void ProcessEvent(const PlayerScoreUpdatedEvent& event) {
        // Можно добавить дополнительную логику если нужно
    }

    mutable std::mutex mutex_;
    std::map<boost::uuids::uuid, GameSessionStats> stats_;
};

} // namespace game_userver::logic::game::observers
