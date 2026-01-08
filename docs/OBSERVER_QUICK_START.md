# Observer Pattern - Краткое руководство

Быстрый старт для использования системы Observer в игровом бэкенде.

## Что это?

Observer Pattern позволяет выполнять действия при игровых событиях:
- 📝 Логирование
- 📊 Сбор статистики
- 📢 Уведомления администратору
- 💬 Отправка сообщений игрокам
- 📈 Метрики и мониторинг

## Быстрый старт

### 1. Использование встроенных Observer'ов

```cpp
#include "logic/game/game.hpp"
#include "logic/game/observers/logging_observer.hpp"
#include "logic/game/observers/statistics_observer.hpp"
#include "logic/game/observers/notification_observer.hpp"

// Создаём GameService
logic::game::GameService game_service(pg_cluster);

// Добавляем логирование
game_service.AddObserver(
    std::make_shared<observers::LoggingObserver>()
);

// Добавляем сбор статистики
auto stats_observer = std::make_shared<observers::StatisticsObserver>();
game_service.AddObserver(stats_observer);

// Добавляем уведомления администратору
game_service.AddObserver(
    std::make_shared<observers::AdminNotificationObserver>()
);
```

### 2. Создание простого Observer'а

```cpp
#include "logic/game/game_observer.hpp"

class MyObserver : public game_userver::logic::game::IGameObserver {
public:
    void OnEvent(const GameEvent& event) override {
        std::visit([](const auto& e) {
            // Обработка события
            LOG_INFO() << "Event received!";
        }, event);
    }
};

// Использование
game_service.AddObserver(std::make_shared<MyObserver>());
```

### 3. Observer с фильтрацией событий

```cpp
class AnswerObserver : public IGameObserver {
public:
    void OnEvent(const GameEvent& event) override {
        if (auto* e = std::get_if<AnswerSubmittedEvent>(&event)) {
            LOG_INFO() << "Player " << e->player_name 
                       << " answered: " 
                       << (e->is_correct ? "CORRECT" : "INCORRECT");
        }
    }

    bool ShouldHandleEvent(GameEventType type) const override {
        return type == GameEventType::kAnswerSubmitted;
    }
};
```

## Типы событий

| Событие | Когда происходит |
|---------|------------------|
| `GameSessionCreated` | Создана игровая сессия |
| `PlayerAdded` | Добавлен игрок |
| `GameStarted` | Игра началась |
| `QuestionPresented` | Показан вопрос |
| `AnswerSubmitted` | Игрок отправил ответ |
| `AllPlayersAnswered` | Все игроки ответили |
| `QuestionAdvanced` | Переход к следующему вопросу |
| `GameFinished` | Игра завершена |
| `PlayerScoreUpdated` | Обновлён счёт игрока |

## Примеры сценариев

### Уведомление администратора о числе ответивших игроков

```cpp
class AdminAnswerNotifier : public IGameObserver {
public:
    void OnEvent(const GameEvent& event) override {
        if (auto* e = std::get_if<AllPlayersAnsweredEvent>(&event)) {
            LOG_WARNING() << "[ADMIN] " << e->total_players 
                         << " players answered question " 
                         << (e->question_index + 1);
        }
    }

    bool ShouldHandleEvent(GameEventType type) const override {
        return type == GameEventType::kAllPlayersAnswered;
    }
};
```

### Уведомление игроков о начале игры

```cpp
class GameStartNotifier : public IGameObserver {
public:
    using NotifyFunc = std::function<void(const std::string&)>;
    
    GameStartNotifier(NotifyFunc notify) : notify_(notify) {}

    void OnEvent(const GameEvent& event) override {
        if (auto* e = std::get_if<GameStartedEvent>(&event)) {
            notify_("🎮 Игра началась! Участников: " + 
                   std::to_string(e->total_players));
        }
    }

    bool ShouldHandleEvent(GameEventType type) const override {
        return type == GameEventType::kGameStarted;
    }

private:
    NotifyFunc notify_;
};

// Использование
game_service.AddObserver(
    std::make_shared<GameStartNotifier>(
        [](const std::string& msg) {
            // Отправка уведомления всем игрокам
            SendToAllPlayers(msg);
        }
    )
);
```

### Уведомление игрока о принятии ответа

```cpp
class AnswerAcceptedNotifier : public IGameObserver {
public:
    using PlayerNotifyFunc = 
        std::function<void(const boost::uuids::uuid&, const std::string&)>;
    
    AnswerAcceptedNotifier(PlayerNotifyFunc notify) : notify_(notify) {}

    void OnEvent(const GameEvent& event) override {
        if (auto* e = std::get_if<AnswerSubmittedEvent>(&event)) {
            std::string msg = e->is_correct 
                ? "✅ Ответ принят! Правильно!" 
                : "❌ Ответ принят. Неправильно.";
            notify_(e->player_id, msg);
        }
    }

    bool ShouldHandleEvent(GameEventType type) const override {
        return type == GameEventType::kAnswerSubmitted;
    }

private:
    PlayerNotifyFunc notify_;
};

// Использование
game_service.AddObserver(
    std::make_shared<AnswerAcceptedNotifier>(
        [](const boost::uuids::uuid& player_id, const std::string& msg) {
            // Отправка уведомления конкретному игроку
            SendToPlayer(player_id, msg);
        }
    )
);
```

## Получение статистики

```cpp
// Создаём StatisticsObserver
auto stats_observer = std::make_shared<observers::StatisticsObserver>();
game_service.AddObserver(stats_observer);

// После игры получаем статистику
auto stats = stats_observer->GetStats(game_session_id);
if (stats) {
    std::cout << "Игроков: " << stats->total_players << "\n";
    std::cout << "Вопросов: " << stats->total_questions << "\n";
    std::cout << "Правильных ответов: " << stats->correct_answers << "\n";
    std::cout << "Неправильных ответов: " << stats->incorrect_answers << "\n";
}
```

## Интеграция с Telegram

```cpp
class TelegramObserver : public IGameObserver {
public:
    TelegramObserver(const std::string& bot_token, const std::string& chat_id)
        : bot_token_(bot_token), chat_id_(chat_id) {}

    void OnEvent(const GameEvent& event) override {
        std::visit([this](const auto& e) { ProcessEvent(e); }, event);
    }

private:
    void ProcessEvent(const GameStartedEvent& event) {
        SendTelegramMessage("🎮 Игра началась!");
    }

    void ProcessEvent(const AllPlayersAnsweredEvent& event) {
        SendTelegramMessage("✅ Все ответили на вопрос " + 
                           std::to_string(event.question_index + 1));
    }

    void ProcessEvent(const GameFinishedEvent& event) {
        SendTelegramMessage("🏁 Игра завершена!");
    }

    template<typename T>
    void ProcessEvent(const T&) {}

    void SendTelegramMessage(const std::string& text) {
        // Реализация отправки в Telegram
        // Используйте библиотеку для работы с Telegram Bot API
    }

    std::string bot_token_;
    std::string chat_id_;
};
```

## Управление Observer'ами

```cpp
// Добавить
auto observer = std::make_shared<MyObserver>();
game_service.AddObserver(observer);

// Удалить
game_service.RemoveObserver(observer);

// Очистить все
game_service.ClearObservers();

// Получить количество
auto& manager = game_service.GetObserverManager();
size_t count = manager.GetObserverCount();
```

## Советы

1. ✅ **Используйте фильтрацию** - Реализуйте `ShouldHandleEvent()` для обработки только нужных событий
2. ✅ **Обрабатывайте ошибки** - Используйте try-catch в Observer'ах
3. ✅ **Логируйте проблемы** - Всегда логируйте ошибки
4. ❌ **Избегайте тяжёлых операций** - Не блокируйте основной поток
5. ❌ **Не изменяйте состояние игры** - Observer только наблюдает

## Полная документация

Подробная документация: [OBSERVER_PATTERN.md](OBSERVER_PATTERN.md)

## Примеры кода

Больше примеров в файле: `src/logic/game/observers/examples.hpp`