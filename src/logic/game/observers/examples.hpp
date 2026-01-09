#pragma once

#include "../game_observer.hpp"
#include "logging_observer.hpp"
#include "notification_observer.hpp"
#include "statistics_observer.hpp"

#include <boost/uuid/uuid_io.hpp>
#include <userver/logging/log.hpp>

namespace game_userver::logic::game::observers {

// Пример: Observer для отслеживания прогресса игры
class GameProgressObserver : public IGameObserver {
public:
    void OnEvent(const GameEvent& event) override {
        std::visit(
            [this](const auto& e) {
                TrackProgress(e);
            },
            event
        );
    }

    bool ShouldHandleEvent(GameEventType type) const override {
        return type == GameEventType::kQuestionPresented ||
               type == GameEventType::kAllPlayersAnswered ||
               type == GameEventType::kGameFinished;
    }

private:
    void TrackProgress(const GameSessionCreatedEvent& event) {}
    void TrackProgress(const PlayerAddedEvent& event) {}
    void TrackProgress(const GameStartedEvent& event) {}

    void TrackProgress(const QuestionPresentedEvent& event) {
        LOG_INFO() << "[PROGRESS] Question " << (event.question_index + 1)
                   << "/" << event.total_questions << " presented in game "
                   << boost::uuids::to_string(event.game_session_id);
    }

    void TrackProgress(const AnswerSubmittedEvent& event) {}

    void TrackProgress(const AllPlayersAnsweredEvent& event) {
        LOG_INFO() << "[PROGRESS] All players answered question "
                   << (event.question_index + 1) << " in game "
                   << boost::uuids::to_string(event.game_session_id);
    }

    void TrackProgress(const QuestionAdvancedEvent& event) {}

    void TrackProgress(const GameFinishedEvent& event) {
        LOG_INFO() << "[PROGRESS] Game finished: "
                   << boost::uuids::to_string(event.game_session_id) << " with "
                   << event.total_questions << " questions";
    }

    void TrackProgress(const PlayerScoreUpdatedEvent& event) {}
};

// Пример: Observer для отправки уведомлений в Telegram
class TelegramNotificationObserver : public IGameObserver {
public:
    using TelegramSender = std::function<
        void(const std::string& chat_id, const std::string& message)>;

    TelegramNotificationObserver(
        TelegramSender sender, const std::string& admin_chat_id
    )
        : sender_(std::move(sender)), admin_chat_id_(admin_chat_id) {}

    void OnEvent(const GameEvent& event) override {
        std::visit(
            [this](const auto& e) {
                SendTelegramNotification(e);
            },
            event
        );
    }

    bool ShouldHandleEvent(GameEventType type) const override {
        return type == GameEventType::kGameStarted ||
               type == GameEventType::kAllPlayersAnswered ||
               type == GameEventType::kGameFinished;
    }

private:
    void SendTelegramNotification(const GameSessionCreatedEvent& event) {}
    void SendTelegramNotification(const PlayerAddedEvent& event) {}

    void SendTelegramNotification(const GameStartedEvent& event) {
        std::string message = "🎮 Игра началась!\n";
        message += "Игроков: " + std::to_string(event.total_players) + "\n";
        message += "Вопросов: " + std::to_string(event.total_questions);
        SendMessage(message);
    }

    void SendTelegramNotification(const QuestionPresentedEvent& event) {}
    void SendTelegramNotification(const AnswerSubmittedEvent& event) {}

    void SendTelegramNotification(const AllPlayersAnsweredEvent& event) {
        std::string message = "✅ Все игроки ответили на вопрос " +
                              std::to_string(event.question_index + 1);
        SendMessage(message);
    }

    void SendTelegramNotification(const QuestionAdvancedEvent& event) {}

    void SendTelegramNotification(const GameFinishedEvent& event) {
        std::string message = "🏁 Игра завершена!\n";
        message +=
            "Всего вопросов: " + std::to_string(event.total_questions) + "\n";
        message += "Игроков: " + std::to_string(event.total_players);
        SendMessage(message);
    }

    void SendTelegramNotification(const PlayerScoreUpdatedEvent& event) {}

    void SendMessage(const std::string& message) {
        if (sender_) {
            try {
                sender_(admin_chat_id_, message);
            } catch (const std::exception& e) {
                LOG_ERROR()
                    << "Failed to send Telegram notification: " << e.what();
            }
        }
    }

    TelegramSender sender_;
    std::string admin_chat_id_;
};

// Пример: Observer для подсчёта метрик
class MetricsObserver : public IGameObserver {
public:
    void OnEvent(const GameEvent& event) override {
        std::visit(
            [this](const auto& e) {
                CollectMetrics(e);
            },
            event
        );
    }

    struct Metrics {
        int total_games_created = 0;
        int total_games_started = 0;
        int total_games_finished = 0;
        int total_players_added = 0;
        int total_answers_submitted = 0;
        int total_correct_answers = 0;
        int total_incorrect_answers = 0;
    };

    [[nodiscard]] const Metrics& GetMetrics() const {
        return metrics_;
    }

    void ResetMetrics() {
        metrics_ = Metrics{};
    }

private:
    void CollectMetrics(const GameSessionCreatedEvent& event) {
        metrics_.total_games_created++;
    }

    void CollectMetrics(const PlayerAddedEvent& event) {
        metrics_.total_players_added++;
    }

    void CollectMetrics(const GameStartedEvent& event) {
        metrics_.total_games_started++;
    }

    void CollectMetrics(const QuestionPresentedEvent& event) {}

    void CollectMetrics(const AnswerSubmittedEvent& event) {
        metrics_.total_answers_submitted++;
        if (event.is_correct) {
            metrics_.total_correct_answers++;
        } else {
            metrics_.total_incorrect_answers++;
        }
    }

    void CollectMetrics(const AllPlayersAnsweredEvent& event) {}
    void CollectMetrics(const QuestionAdvancedEvent& event) {}

    void CollectMetrics(const GameFinishedEvent& event) {
        metrics_.total_games_finished++;
    }

    void CollectMetrics(const PlayerScoreUpdatedEvent& event) {}

    Metrics metrics_;
};

} // namespace game_userver::logic::game::observers
