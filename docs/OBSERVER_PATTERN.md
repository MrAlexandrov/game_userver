# Observer Pattern для игровых событий

Этот документ описывает систему Observer, реализованную для отслеживания игровых событий в бэкенде игры.

## Обзор

Паттерн Observer позволяет выполнять различные действия при определённых игровых событиях без изменения основной логики игры. Это полезно для:

- Логирования игровых событий
- Отправки уведомлений администраторам
- Сбора статистики и метрик
- Интеграции с внешними системами (Telegram, email и т.д.)
- Отслеживания прогресса игры

## Архитектура

### Основные компоненты

1. **GameEvent** - Вариант (std::variant) всех типов игровых событий
2. **IGameObserver** - Базовый интерфейс для всех наблюдателей
3. **GameObserverManager** - Менеджер для управления наблюдателями
4. **GameService** - Интегрирован с системой Observer

### Типы событий

```cpp
enum class GameEventType {
    kGameSessionCreated,      // Создана игровая сессия
    kPlayerAdded,             // Добавлен игрок
    kGameStarted,             // Игра началась
    kQuestionPresented,       // Представлен вопрос
    kAnswerSubmitted,         // Отправлен ответ
    kAllPlayersAnswered,      // Все игроки ответили
    kQuestionAdvanced,        // Переход к следующему вопросу
    kGameFinished,            // Игра завершена
    kPlayerScoreUpdated       // Обновлён счёт игрока
};
```

## Встроенные Observer'ы

### 1. LoggingObserver

Логирует все игровые события в консоль.

```cpp
#include "logic/game/observers/logging_observer.hpp"

auto logging_observer = std::make_shared<observers::LoggingObserver>();
game_service.AddObserver(logging_observer);
```

**Пример вывода:**
```
[INFO] Game session created: 550e8400-e29b-41d4-a716-446655440000
[INFO] Player added: Алексей (id: 770e8400-...) to game 550e8400-...
[INFO] Game started: 550e8400-..., players: 2, questions: 3
[INFO] Answer submitted by Алексей: CORRECT in game 550e8400-...
```

### 2. StatisticsObserver

Собирает статистику по игровым сессиям.

```cpp
#include "logic/game/observers/statistics_observer.hpp"

auto stats_observer = std::make_shared<observers::StatisticsObserver>();
game_service.AddObserver(stats_observer);

// Получить статистику
auto stats = stats_observer->GetStats(game_session_id);
if (stats) {
    LOG_INFO() << "Total players: " << stats->total_players;
    LOG_INFO() << "Questions answered: " << stats->questions_answered;
    LOG_INFO() << "Correct answers: " << stats->correct_answers;
}
```

**Доступные метрики:**
- `total_players` - Количество игроков
- `total_questions` - Общее количество вопросов
- `questions_answered` - Количество отвеченных вопросов
- `total_answers_submitted` - Всего отправлено ответов
- `correct_answers` - Правильных ответов
- `incorrect_answers` - Неправильных ответов
- `is_finished` - Завершена ли игра

### 3. NotificationObserver

Отправляет уведомления при определённых событиях.

```cpp
#include "logic/game/observers/notification_observer.hpp"

auto notification_observer = std::make_shared<observers::NotificationObserver>(
    [](const observers::Notification& notification) {
        // Ваша логика отправки уведомлений
        LOG_WARNING() << "[NOTIFICATION] " << notification.message;
    }
);
game_service.AddObserver(notification_observer);
```

### 4. AdminNotificationObserver

Специализированный Observer для уведомлений администратора.

```cpp
#include "logic/game/observers/notification_observer.hpp"

auto admin_observer = std::make_shared<observers::AdminNotificationObserver>();
game_service.AddObserver(admin_observer);
```

## Создание собственного Observer

### Простой Observer

```cpp
#include "logic/game/game_observer.hpp"

class MyCustomObserver : public game_userver::logic::game::IGameObserver {
public:
    void OnEvent(const GameEvent& event) override {
        std::visit([this](const auto& e) { HandleEvent(e); }, event);
    }

    // Опционально: фильтрация событий
    bool ShouldHandleEvent(GameEventType type) const override {
        return type == GameEventType::kGameStarted ||
               type == GameEventType::kGameFinished;
    }

private:
    void HandleEvent(const GameStartedEvent& event) {
        // Обработка старта игры
        LOG_INFO() << "Custom: Game started with " 
                   << event.total_players << " players";
    }

    void HandleEvent(const GameFinishedEvent& event) {
        // Обработка завершения игры
        LOG_INFO() << "Custom: Game finished";
    }

    // Для остальных событий
    template<typename T>
    void HandleEvent(const T& event) {
        // Игнорируем или обрабатываем по-другому
    }
};
```

### Функциональный Observer

Для простых случаев можно использовать `FunctionalObserver`:

```cpp
auto observer = std::make_shared<FunctionalObserver>(
    [](const GameEvent& event) {
        std::visit([](const auto& e) {
            LOG_INFO() << "Event received at " 
                       << std::chrono::system_clock::now();
        }, event);
    },
    // Опциональный фильтр
    [](GameEventType type) {
        return type == GameEventType::kAnswerSubmitted;
    }
);

game_service.AddObserver(observer);
```

## Примеры использования

### Пример 1: Уведомление администратора о количестве ответивших игроков

```cpp
class AnswerCountObserver : public IGameObserver {
public:
    void OnEvent(const GameEvent& event) override {
        if (auto* e = std::get_if<AllPlayersAnsweredEvent>(&event)) {
            LOG_WARNING() << "[ADMIN] All " << e->total_players 
                         << " players answered question " 
                         << (e->question_index + 1);
        }
    }

    bool ShouldHandleEvent(GameEventType type) const override {
        return type == GameEventType::kAllPlayersAnswered;
    }
};
```

### Пример 2: Отправка уведомлений игрокам о начале игры

```cpp
class GameStartNotifier : public IGameObserver {
public:
    using NotifyFunction = std::function<void(const std::string&)>;
    
    GameStartNotifier(NotifyFunction notify_func) 
        : notify_func_(std::move(notify_func)) {}

    void OnEvent(const GameEvent& event) override {
        if (auto* e = std::get_if<GameStartedEvent>(&event)) {
            std::string message = "Игра началась! Участников: " + 
                                 std::to_string(e->total_players);
            notify_func_(message);
        }
    }

    bool ShouldHandleEvent(GameEventType type) const override {
        return type == GameEventType::kGameStarted;
    }

private:
    NotifyFunction notify_func_;
};
```

### Пример 3: Уведомление игрока о принятии ответа

```cpp
class AnswerAcceptedNotifier : public IGameObserver {
public:
    using PlayerNotifyFunction = 
        std::function<void(const boost::uuids::uuid&, const std::string&)>;
    
    AnswerAcceptedNotifier(PlayerNotifyFunction notify_func)
        : notify_func_(std::move(notify_func)) {}

    void OnEvent(const GameEvent& event) override {
        if (auto* e = std::get_if<AnswerSubmittedEvent>(&event)) {
            std::string message = e->is_correct 
                ? "✅ Ваш ответ принят! Правильно!" 
                : "❌ Ваш ответ принят. Неправильно.";
            notify_func_(e->player_id, message);
        }
    }

    bool ShouldHandleEvent(GameEventType type) const override {
        return type == GameEventType::kAnswerSubmitted;
    }

private:
    PlayerNotifyFunction notify_func_;
};
```

## Интеграция в обработчики

### В существующих обработчиках

Обработчики автоматически используют Observer через `GameService`. Не требуется изменений в коде обработчиков.

### Добавление Observer при инициализации

```cpp
// В main.cpp или в компоненте инициализации
void SetupGameObservers(logic::game::GameService& game_service) {
    // Логирование
    game_service.AddObserver(
        std::make_shared<observers::LoggingObserver>()
    );
    
    // Статистика
    game_service.AddObserver(
        std::make_shared<observers::StatisticsObserver>()
    );
    
    // Уведомления администратора
    game_service.AddObserver(
        std::make_shared<observers::AdminNotificationObserver>()
    );
    
    // Пользовательский Observer
    game_service.AddObserver(
        std::make_shared<MyCustomObserver>()
    );
}
```

## Управление Observer'ами

### Добавление Observer

```cpp
auto observer = std::make_shared<MyObserver>();
game_service.AddObserver(observer);
```

### Удаление Observer

```cpp
game_service.RemoveObserver(observer);
```

### Очистка всех Observer'ов

```cpp
game_service.ClearObservers();
```

### Получение менеджера Observer'ов

```cpp
auto& manager = game_service.GetObserverManager();
size_t count = manager.GetObserverCount();
```

## Производительность

- Observer'ы вызываются синхронно в том же потоке
- Фильтрация событий через `ShouldHandleEvent()` минимизирует накладные расходы
- Используйте фильтрацию для Observer'ов, которые обрабатывают только определённые события

## Лучшие практики

1. **Используйте фильтрацию событий** - Реализуйте `ShouldHandleEvent()` для обработки только нужных событий
2. **Обрабатывайте исключения** - Observer не должен прерывать основную логику игры
3. **Избегайте тяжёлых операций** - Выполняйте длительные операции асинхронно
4. **Логируйте ошибки** - Всегда логируйте ошибки в Observer'ах
5. **Тестируйте Observer'ы** - Создавайте unit-тесты для ваших Observer'ов

## Примеры сценариев

### Сценарий 1: Telegram-бот для администратора

```cpp
class TelegramAdminBot : public IGameObserver {
public:
    TelegramAdminBot(const std::string& bot_token, const std::string& chat_id)
        : bot_token_(bot_token), chat_id_(chat_id) {}

    void OnEvent(const GameEvent& event) override {
        std::visit([this](const auto& e) { SendToTelegram(e); }, event);
    }

private:
    void SendToTelegram(const GameStartedEvent& event) {
        SendMessage("🎮 Новая игра началась!\n"
                   "Игроков: " + std::to_string(event.total_players));
    }

    void SendToTelegram(const AllPlayersAnsweredEvent& event) {
        SendMessage("✅ Все игроки ответили на вопрос " + 
                   std::to_string(event.question_index + 1));
    }

    template<typename T>
    void SendToTelegram(const T& event) {}

    void SendMessage(const std::string& text) {
        // Реализация отправки в Telegram
    }

    std::string bot_token_;
    std::string chat_id_;
};
```

### Сценарий 2: Сбор метрик для мониторинга

```cpp
class PrometheusMetricsObserver : public IGameObserver {
public:
    void OnEvent(const GameEvent& event) override {
        std::visit([this](const auto& e) { UpdateMetrics(e); }, event);
    }

private:
    void UpdateMetrics(const GameSessionCreatedEvent& event) {
        games_created_counter_++;
    }

    void UpdateMetrics(const AnswerSubmittedEvent& event) {
        answers_submitted_counter_++;
        if (event.is_correct) {
            correct_answers_counter_++;
        }
    }

    template<typename T>
    void UpdateMetrics(const T& event) {}

    std::atomic<int> games_created_counter_{0};
    std::atomic<int> answers_submitted_counter_{0};
    std::atomic<int> correct_answers_counter_{0};
};
```

## Заключение

Система Observer предоставляет гибкий способ реагирования на игровые события без изменения основной логики. Вы можете легко добавлять новые Observer'ы для различных сценариев использования.

## См. также

- [GAME_FLOW.md](GAME_FLOW.md) - Процесс игры
- [API_PACK_YAML.md](API_PACK_YAML.md) - API для работы с паками
- Исходный код Observer'ов в `src/logic/game/observers/`