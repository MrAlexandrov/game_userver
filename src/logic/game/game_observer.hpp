#pragma once

#include "game_events.hpp"
#include <functional>
#include <memory>
#include <vector>

namespace game_userver::logic::game {

// Базовый интерфейс Observer
class IGameObserver {
public:
    virtual ~IGameObserver() = default;

    // Метод для обработки событий
    virtual void OnEvent(const GameEvent& event) = 0;

    // Опциональный метод для фильтрации событий
    virtual bool ShouldHandleEvent(GameEventType type) const {
        return true; // По умолчанию обрабатываем все события
    }
};

// Менеджер наблюдателей
class GameObserverManager {
public:
    // Добавить наблюдателя
    void AddObserver(std::shared_ptr<IGameObserver> observer) {
        observers_.push_back(observer);
    }

    // Удалить наблюдателя
    void RemoveObserver(std::shared_ptr<IGameObserver> observer) {
        observers_.erase(
            std::remove(observers_.begin(), observers_.end(), observer),
            observers_.end()
        );
    }

    // Очистить всех наблюдателей
    void ClearObservers() {
        observers_.clear();
    }

    // Уведомить всех наблюдателей о событии
    void NotifyObservers(const GameEvent& event) const {
        GameEventType event_type = std::visit(
            [](const auto& e) {
                return e.type;
            },
            event
        );

        for (const auto& observer : observers_) {
            if (observer && observer->ShouldHandleEvent(event_type)) {
                observer->OnEvent(event);
            }
        }
    }

    // Получить количество наблюдателей
    [[nodiscard]] size_t GetObserverCount() const {
        return observers_.size();
    }

private:
    std::vector<std::shared_ptr<IGameObserver>> observers_;
};

// Вспомогательный класс для создания простых наблюдателей на основе функций
class FunctionalObserver : public IGameObserver {
public:
    using EventHandler = std::function<void(const GameEvent&)>;
    using EventFilter = std::function<bool(GameEventType)>;

    explicit FunctionalObserver(EventHandler handler)
        : handler_(std::move(handler)), filter_(nullptr) {}

    FunctionalObserver(EventHandler handler, EventFilter filter)
        : handler_(std::move(handler)), filter_(std::move(filter)) {}

    void OnEvent(const GameEvent& event) override {
        if (handler_) {
            handler_(event);
        }
    }

    bool ShouldHandleEvent(GameEventType type) const override {
        if (filter_) {
            return filter_(type);
        }
        return true;
    }

private:
    EventHandler handler_;
    EventFilter filter_;
};

} // namespace game_userver::logic::game
