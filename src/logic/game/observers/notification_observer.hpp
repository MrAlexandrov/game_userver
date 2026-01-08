#pragma once

#include "../game_observer.hpp"
#include <userver/logging/log.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <functional>
#include <string>

namespace game_userver::logic::game::observers {

// Типы уведомлений
enum class NotificationType {
    kPlayerJoined,
    kGameStarted,
    kAllPlayersAnswered,
    kGameFinished,
    kPlayerScoreChanged
};

// Структура уведомления
struct Notification {
    NotificationType type;
    boost::uuids::uuid game_session_id;
    std::string message;
    std::chrono::system_clock::time_point timestamp;
};

// Observer для отправки уведомлений
// Может использоваться для уведомления администратора, отправки в Telegram и т.д.
class NotificationObserver : public IGameObserver {
public:
    using NotificationHandler = std::function<void(const Notification&)>;

    explicit NotificationObserver(NotificationHandler handler)
        : handler_(std::move(handler)) {}

    void OnEvent(const GameEvent& event) override {
        std::visit([this](const auto& e) { ProcessEvent(e); }, event);
    }

    bool ShouldHandleEvent(GameEventType type) const override {
        // Обрабатываем только определённые типы событий
        return type == GameEventType::kPlayerAdded ||
               type == GameEventType::kGameStarted ||
               type == GameEventType::kAllPlayersAnswered ||
               type == GameEventType::kGameFinished ||
               type == GameEventType::kPlayerScoreUpdated;
    }

private:
    void ProcessEvent(const GameSessionCreatedEvent& event) {
        // Не отправляем уведомление
    }

    void ProcessEvent(const PlayerAddedEvent& event) {
        Notification notification{
            NotificationType::kPlayerJoined, event.game_session_id,
            "Player '" + event.player.name + "' joined the game",
            event.timestamp};
        SendNotification(notification);
    }

    void ProcessEvent(const GameStartedEvent& event) {
        Notification notification{
            NotificationType::kGameStarted, event.game_session_id,
            "Game started with " + std::to_string(event.total_players) +
                " players and " + std::to_string(event.total_questions) +
                " questions",
            event.timestamp};
        SendNotification(notification);
    }

    void ProcessEvent(const QuestionPresentedEvent& event) {
        // Не отправляем уведомление
    }

    void ProcessEvent(const AnswerSubmittedEvent& event) {
        // Не отправляем уведомление для каждого ответа
        // (используем AllPlayersAnswered вместо этого)
    }

    void ProcessEvent(const AllPlayersAnsweredEvent& event) {
        Notification notification{
            NotificationType::kAllPlayersAnswered, event.game_session_id,
            "All " + std::to_string(event.total_players) +
                " players answered question " +
                std::to_string(event.question_index + 1),
            event.timestamp};
        SendNotification(notification);
    }

    void ProcessEvent(const QuestionAdvancedEvent& event) {
        // Не отправляем уведомление
    }

    void ProcessEvent(const GameFinishedEvent& event) {
        Notification notification{
            NotificationType::kGameFinished, event.game_session_id,
            "Game finished! Total questions: " +
                std::to_string(event.total_questions) +
                ", players: " + std::to_string(event.total_players),
            event.timestamp};
        SendNotification(notification);
    }

    void ProcessEvent(const PlayerScoreUpdatedEvent& event) {
        Notification notification{
            NotificationType::kPlayerScoreChanged, event.game_session_id,
            "Player '" + event.player_name + "' score updated: " +
                std::to_string(event.old_score) + " -> " +
                std::to_string(event.new_score),
            event.timestamp};
        SendNotification(notification);
    }

    void SendNotification(const Notification& notification) {
        if (handler_) {
            try {
                handler_(notification);
            } catch (const std::exception& e) {
                LOG_ERROR() << "Failed to send notification: " << e.what();
            }
        }
    }

    NotificationHandler handler_;
};

// Простой Observer для логирования уведомлений администратору
class AdminNotificationObserver : public NotificationObserver {
public:
    AdminNotificationObserver()
        : NotificationObserver([](const Notification& notification) {
              LOG_WARNING() << "[ADMIN NOTIFICATION] "
                            << "Game: "
                            << boost::uuids::to_string(
                                   notification.game_session_id
                               )
                            << " - " << notification.message;
          }) {}
};

} // namespace game_userver::logic::game::observers