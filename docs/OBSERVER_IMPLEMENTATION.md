# Реализация Observer Pattern - Техническая документация

Этот документ описывает техническую реализацию паттерна Observer в игровом бэкенде.

## Архитектура

### Структура файлов

```
src/logic/game/
├── game_events.hpp              # Определения всех типов событий
├── game_observer.hpp            # Базовый интерфейс и менеджер Observer
├── game.hpp                     # GameService с интеграцией Observer
├── game.cpp                     # Реализация с уведомлениями
└── observers/
    ├── logging_observer.hpp     # Observer для логирования
    ├── statistics_observer.hpp  # Observer для сбора статистики
    ├── notification_observer.hpp # Observer для уведомлений
    └── examples.hpp             # Примеры реализации Observer

docs/
├── OBSERVER_PATTERN.md          # Полная документация
├── OBSERVER_QUICK_START.md      # Краткое руководство
└── OBSERVER_IMPLEMENTATION.md   # Этот файл
```

## Компоненты системы

### 1. GameEvent (game_events.hpp)

Определяет все типы событий в игре:

```cpp
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
```

Каждое событие имеет свою структуру с релевантными данными:

- **GameSessionCreatedEvent** - ID сессии, ID пака
- **PlayerAddedEvent** - Данные игрока
- **GameStartedEvent** - Количество игроков и вопросов
- **QuestionPresentedEvent** - Вопрос, индекс, общее количество
- **AnswerSubmittedEvent** - ID игрока, вопроса, варианта, правильность
- **AllPlayersAnsweredEvent** - ID вопроса, количество ответов
- **QuestionAdvancedEvent** - Предыдущий и новый индекс
- **GameFinishedEvent** - Итоговая статистика
- **PlayerScoreUpdatedEvent** - Старый и новый счёт

### 2. IGameObserver (game_observer.hpp)

Базовый интерфейс для всех Observer'ов:

```cpp
class IGameObserver {
public:
    virtual ~IGameObserver() = default;
    virtual void OnEvent(const GameEvent& event) = 0;
    virtual bool ShouldHandleEvent(GameEventType type) const {
        return true;
    }
};
```

### 3. GameObserverManager (game_observer.hpp)

Управляет коллекцией Observer'ов:

```cpp
class GameObserverManager {
public:
    void AddObserver(std::shared_ptr<IGameObserver> observer);
    void RemoveObserver(std::shared_ptr<IGameObserver> observer);
    void ClearObservers();
    void NotifyObservers(const GameEvent& event) const;
    size_t GetObserverCount() const;
};
```

### 4. GameService Integration (game.hpp, game.cpp)

GameService интегрирован с системой Observer:

```cpp
class GameService {
public:
    void AddObserver(std::shared_ptr<IGameObserver> observer);
    void RemoveObserver(std::shared_ptr<IGameObserver> observer);
    void ClearObservers();
    GameObserverManager& GetObserverManager();

private:
    void NotifyObservers(const GameEvent& event);
    GameObserverManager observer_manager_;
};
```

## Точки уведомления

Observer'ы уведомляются в следующих точках:

### CreateGameSession
```cpp
auto game_session = NStorage::CreateGameSession(pg_cluster_, pack_id);
if (game_session) {
    NotifyObservers(GameSessionCreatedEvent(game_session->id, pack_id));
}
```

### AddPlayer
```cpp
auto player = NStorage::AddPlayer(pg_cluster_, game_session_id, player_name);
if (player) {
    NotifyObservers(PlayerAddedEvent(game_session_id, *player));
}
```

### StartGame
```cpp
auto game_session = NStorage::StartGameSession(pg_cluster_, game_session_id);
if (game_session) {
    // Уведомление о старте
    NotifyObservers(GameStartedEvent(...));
    
    // Уведомление о первом вопросе
    NotifyObservers(QuestionPresentedEvent(...));
}
```

### SubmitAnswer
```cpp
// После отправки ответа
NotifyObservers(AnswerSubmittedEvent(...));

// После обновления счёта
if (is_correct && updated_player) {
    NotifyObservers(PlayerScoreUpdatedEvent(...));
}

// Когда все ответили
if (all_players_answered) {
    NotifyObservers(AllPlayersAnsweredEvent(...));
}

// При переходе к следующему вопросу
if (!is_last_question && all_players_answered) {
    NotifyObservers(QuestionAdvancedEvent(...));
    NotifyObservers(QuestionPresentedEvent(...));
}

// При завершении игры
if (is_last_question && all_players_answered) {
    NotifyObservers(GameFinishedEvent(...));
}
```

## Встроенные Observer'ы

### LoggingObserver

Логирует все события в консоль с использованием userver logging:

```cpp
class LoggingObserver : public IGameObserver {
    void OnEvent(const GameEvent& event) override {
        std::visit([this](const auto& e) { LogEvent(e); }, event);
    }
private:
    void LogEvent(const GameStartedEvent& event) {
        LOG_INFO() << "Game started: " << event.game_session_id
                   << ", players: " << event.total_players;
    }
    // ... другие методы LogEvent
};
```

### StatisticsObserver

Собирает статистику по игровым сессиям:

```cpp
struct GameSessionStats {
    int total_players = 0;
    int total_questions = 0;
    int questions_answered = 0;
    int total_answers_submitted = 0;
    int correct_answers = 0;
    int incorrect_answers = 0;
    bool is_finished = false;
};

class StatisticsObserver : public IGameObserver {
    std::map<boost::uuids::uuid, GameSessionStats> stats_;
    mutable std::mutex mutex_;
};
```

### NotificationObserver

Отправляет уведомления через callback-функцию:

```cpp
class NotificationObserver : public IGameObserver {
public:
    using NotificationHandler = std::function<void(const Notification&)>;
    
    NotificationObserver(NotificationHandler handler);
    
    bool ShouldHandleEvent(GameEventType type) const override {
        // Фильтрует только важные события
        return type == GameEventType::kPlayerAdded ||
               type == GameEventType::kGameStarted ||
               type == GameEventType::kAllPlayersAnswered ||
               type == GameEventType::kGameFinished;
    }
};
```

## Потокобезопасность

### StatisticsObserver
- Использует `std::mutex` для защиты `stats_`
- Все операции чтения/записи защищены `std::lock_guard`

### GameObserverManager
- Не является потокобезопасным по умолчанию
- Предполагается, что Observer'ы добавляются/удаляются при инициализации
- Уведомления происходят в том же потоке, что и игровая логика

## Производительность

### Оптимизации

1. **Фильтрация событий** - `ShouldHandleEvent()` позволяет избежать ненужных вызовов
2. **Синхронное выполнение** - Нет накладных расходов на многопоточность
3. **std::visit** - Эффективная диспетчеризация событий
4. **std::shared_ptr** - Управление временем жизни Observer'ов

### Рекомендации

- Избегайте тяжёлых операций в Observer'ах
- Используйте фильтрацию для обработки только нужных событий
- Для длительных операций используйте асинхронное выполнение
- Логируйте ошибки, но не прерывайте основной поток

## Расширяемость

### Добавление нового типа события

1. Добавить новый тип в `GameEventType`
2. Создать структуру события, наследующую `GameEventBase`
3. Добавить тип в `GameEvent` variant
4. Добавить уведомление в соответствующее место в `GameService`
5. Обновить существующие Observer'ы при необходимости

Пример:
```cpp
// 1. В GameEventType
enum class GameEventType {
    // ...
    kPlayerDisconnected  // Новый тип
};

// 2. Структура события
struct PlayerDisconnectedEvent : GameEventBase {
    boost::uuids::uuid player_id;
    std::string reason;
    
    PlayerDisconnectedEvent(
        const boost::uuids::uuid& session_id,
        const boost::uuids::uuid& player_id_val,
        const std::string& reason_val
    ) : GameEventBase(GameEventType::kPlayerDisconnected, session_id),
        player_id(player_id_val),
        reason(reason_val) {}
};

// 3. Добавить в variant
using GameEvent = std::variant<
    // ...
    PlayerDisconnectedEvent
>;

// 4. Уведомление в GameService
void GameService::HandlePlayerDisconnect(const uuid& player_id) {
    // ... логика
    NotifyObservers(PlayerDisconnectedEvent(session_id, player_id, reason));
}
```

### Создание нового Observer'а

1. Наследоваться от `IGameObserver`
2. Реализовать `OnEvent()`
3. Опционально реализовать `ShouldHandleEvent()`
4. Добавить в `GameService` при инициализации

## Тестирование

### Unit-тесты для Observer'ов

```cpp
TEST(ObserverTest, LoggingObserver) {
    auto observer = std::make_shared<LoggingObserver>();
    
    GameStartedEvent event(game_id, 2, 3);
    observer->OnEvent(event);
    
    // Проверить логи
}

TEST(ObserverTest, StatisticsObserver) {
    auto observer = std::make_shared<StatisticsObserver>();
    
    // Отправить события
    observer->OnEvent(GameStartedEvent(...));
    observer->OnEvent(AnswerSubmittedEvent(...));
    
    // Проверить статистику
    auto stats = observer->GetStats(game_id);
    ASSERT_TRUE(stats.has_value());
    EXPECT_EQ(stats->total_answers_submitted, 1);
}
```

### Интеграционные тесты

```cpp
TEST(GameServiceTest, ObserverNotifications) {
    GameService service(pg_cluster);
    
    auto mock_observer = std::make_shared<MockObserver>();
    service.AddObserver(mock_observer);
    
    // Выполнить игровые действия
    service.CreateGameSession(pack_id);
    
    // Проверить, что Observer был уведомлён
    EXPECT_TRUE(mock_observer->WasNotified());
}
```

## Отладка

### Логирование событий

Используйте `LoggingObserver` для отладки:

```cpp
game_service.AddObserver(std::make_shared<LoggingObserver>());
```

### Подсчёт событий

```cpp
class EventCounterObserver : public IGameObserver {
    std::map<GameEventType, int> counts_;
public:
    void OnEvent(const GameEvent& event) override {
        auto type = std::visit([](const auto& e) { return e.type; }, event);
        counts_[type]++;
    }
    
    void PrintCounts() {
        for (const auto& [type, count] : counts_) {
            LOG_INFO() << "Event type " << static_cast<int>(type) 
                       << ": " << count;
        }
    }
};
```

## Заключение

Система Observer предоставляет гибкий и расширяемый способ реагирования на игровые события. Она легко интегрируется с существующим кодом и позволяет добавлять новую функциональность без изменения основной логики игры.

## См. также

- [OBSERVER_PATTERN.md](OBSERVER_PATTERN.md) - Полная документация
- [OBSERVER_QUICK_START.md](OBSERVER_QUICK_START.md) - Краткое руководство
- [GAME_FLOW.md](GAME_FLOW.md) - Процесс игры