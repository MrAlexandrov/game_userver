# Поток данных и сценарии использования

Документ описывает как данные проходят через систему в различных сценариях использования.

---

## Содержание

1. [Основные потоки данных](#основные-потоки-данных)
2. [Сценарии использования](#сценарии-использования)
3. [Диаграммы последовательности](#диаграммы-последовательности)
4. [Состояния и переходы](#состояния-и-переходы)
5. [Обработка событий](#обработка-событий)

---

## Основные потоки данных

### 1. Создание контента (Content Creation Flow)

```
┌──────────────┐
│   Admin      │
└──────┬───────┘
       │ POST /packs/yaml
       ▼
┌─────────────────────────────────────────┐
│  CreatePackFromYaml Handler             │
│  1. Парсинг YAML                        │
│  2. Валидация структуры                 │
└──────┬──────────────────────────────────┘
       │
       ▼
┌─────────────────────────────────────────┐
│  Storage Layer                          │
│  1. BEGIN TRANSACTION                   │
│  2. INSERT INTO packs                   │
│  3. INSERT INTO questions (batch)       │
│  4. INSERT INTO variants (batch)        │
│  5. COMMIT                              │
└──────┬──────────────────────────────────┘
       │
       ▼
┌─────────────────────────────────────────┐
│  PostgreSQL                             │
│  - packs table                          │
│  - questions table                      │
│  - variants table                       │
└─────────────────────────────────────────┘
```

**Ключевые моменты**:
- Все операции в одной транзакции
- Batch insert для производительности
- Валидация на уровне handler

---

### 2. Игровой процесс (Game Flow)

```
┌──────────────┐
│   Player 1   │
└──────┬───────┘
       │ POST /games/{id}/answers
       ▼
┌─────────────────────────────────────────┐
│  SubmitAnswer Handler                   │
│  1. Извлечение player_id, variant_id    │
│  2. Валидация входных данных            │
└──────┬──────────────────────────────────┘
       │ impl_->game_service.SubmitAnswer()
       ▼
┌─────────────────────────────────────────┐
│  GameService (Singleton)                │
│  1. GetPlayerById()                     │
│  2. GetGameSessionById()                │
│  3. GetCurrentQuestion()                │
│  4. ValidateAnswer()                    │
│  5. SubmitPlayerAnswer()                │
│  6. UpdatePlayerScore() if correct      │
│  7. NotifyObservers()                   │
│  8. CheckAllPlayersAnswered()           │
│  9. AdvanceToNextQuestion() if needed   │
│  10. EndGameSession() if last question  │
└──────┬──────────────────────────────────┘
       │
       ├──────────────────────────────────┐
       │                                  │
       ▼                                  ▼
┌──────────────────┐            ┌──────────────────┐
│  Storage Layer   │            │  Observers       │
│  - SQL queries   │            │  - Logging       │
│  - Transactions  │            │  - Notifications │
└──────┬───────────┘            │  - Statistics    │
       │                        └──────────────────┘
       ▼
┌─────────────────────────────────────────┐
│  PostgreSQL                             │
│  - player_answers table                 │
│  - players table (score update)         │
│  - game_sessions table (state update)   │
└─────────────────────────────────────────┘
```

**Ключевые моменты**:
- GameService - singleton, сохраняет состояние
- Observers уведомляются о каждом событии
- Автоматический переход к следующему вопросу
- Автоматическое завершение игры

---

### 3. Real-time обновления (Observer Pattern)

```
┌─────────────────────────────────────────┐
│  GameService                            │
│  NotifyObservers(AnswerSubmittedEvent)  │
└──────┬──────────────────────────────────┘
       │
       ├─────────────────┬─────────────────┬──────────────────┐
       │                 │                 │                  │
       ▼                 ▼                 ▼                  ▼
┌──────────────┐  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐
│   Logging    │  │ Notification │  │  Statistics  │  │   Custom     │
│   Observer   │  │   Observer   │  │   Observer   │  │   Observer   │
└──────┬───────┘  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘
       │                 │                 │                  │
       ▼                 ▼                 ▼                  ▼
┌──────────────┐  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐
│   Logs       │  │  WebSocket   │  │   Metrics    │  │   External   │
│   Files      │  │   /SSE       │  │   DB         │  │   Service    │
└──────────────┘  └──────────────┘  └──────────────┘  └──────────────┘
```

**Типы событий**:
- `GameSessionCreatedEvent`
- `PlayerAddedEvent`
- `GameStartedEvent`
- `QuestionPresentedEvent`
- `AnswerSubmittedEvent`
- `PlayerScoreUpdatedEvent`
- `AllPlayersAnsweredEvent`
- `QuestionAdvancedEvent`
- `GameFinishedEvent`

---

## Сценарии использования

### Сценарий 1: Создание и проведение простой игры

**Участники**: 1 администратор, 2 игрока

**Шаги**:

1. **Администратор создает пак вопросов**
   ```bash
   POST /packs/yaml
   ```
   - Система создает пак с 3 вопросами
   - Возвращает `pack_id`

2. **Администратор создает игровую сессию**
   ```bash
   POST /games
   Body: { "pack_id": "..." }
   ```
   - Система создает сессию в состоянии `waiting`
   - Возвращает `game_id`

3. **Игрок 1 присоединяется**
   ```bash
   POST /games/{game_id}/players
   Body: { "player_name": "Алексей" }
   ```
   - Система добавляет игрока
   - Observer логирует событие
   - Возвращает `player_1_id`

4. **Игрок 2 присоединяется**
   ```bash
   POST /games/{game_id}/players
   Body: { "player_name": "Мария" }
   ```
   - Система добавляет игрока
   - Возвращает `player_2_id`

5. **Администратор запускает игру**
   ```bash
   POST /games/{game_id}/start
   ```
   - Состояние меняется на `active`
   - Observer отправляет уведомления игрокам
   - `current_question_index = 0`

6. **Игроки получают текущий вопрос**
   ```bash
   GET /games/{game_id}/state
   ```
   - Возвращает вопрос #1 и варианты ответов
   - БЕЗ поля `is_correct` в вариантах

7. **Игрок 1 отвечает**
   ```bash
   POST /games/{game_id}/answers
   Body: { "player_id": "...", "variant_id": "..." }
   ```
   - Система валидирует ответ
   - Обновляет счет если правильно
   - Observer уведомляет о событии
   - Ждет ответа игрока 2

8. **Игрок 2 отвечает**
   ```bash
   POST /games/{game_id}/answers
   Body: { "player_id": "...", "variant_id": "..." }
   ```
   - Система валидирует ответ
   - Обновляет счет если правильно
   - **Все игроки ответили!**
   - Автоматически переходит к вопросу #2
   - `current_question_index = 1`
   - Observer уведомляет о переходе

9. **Повторение шагов 6-8 для вопросов #2 и #3**

10. **После последнего вопроса**
    - Состояние меняется на `finished`
    - Observer отправляет финальные результаты

11. **Получение результатов**
    ```bash
    GET /games/{game_id}/results
    ```
    - Возвращает финальную таблицу с рангами

---

### Сценарий 2: Игра с опозданием игрока

**Проблема**: Игрок присоединяется после начала игры

**Решение**: 
- Игрок НЕ может присоединиться после старта
- Endpoint `POST /games/{game_id}/players` возвращает ошибку если `state != waiting`

**Альтернатива** (для будущей реализации):
- Разрешить присоединение с пропуском пройденных вопросов
- Игрок начинает с текущего вопроса
- Пропущенные вопросы считаются неправильными

---

### Сценарий 3: Игра с отключением игрока

**Проблема**: Игрок отключился и не отвечает

**Текущее поведение**:
- Игра ждет ответа всех игроков
- Другие игроки не могут перейти к следующему вопросу
- **Deadlock!**

**Решение** (TODO):
1. **Timeout на ответ**
   ```typescript
   // После 30 секунд без ответа
   if (time_since_question_presented > 30s) {
     auto_submit_empty_answer(player_id);
   }
   ```

2. **Skip механизм**
   ```bash
   POST /games/{game_id}/skip
   Body: { "player_id": "..." }
   ```
   - Игрок может пропустить вопрос
   - Считается как неправильный ответ

3. **Kick механизм** (для администратора)
   ```bash
   POST /games/{game_id}/kick
   Body: { "player_id": "..." }
   ```
   - Удаляет игрока из активной игры
   - Его ответы сохраняются

---

### Сценарий 4: Множественные игры одновременно

**Ситуация**: 10 игровых сессий идут параллельно

**Как работает**:

```
┌─────────────────────────────────────────┐
│  GameServiceComponent (Singleton)       │
│                                         │
│  ┌───────────────────────────────────┐ │
│  │  GameService                      │ │
│  │  - pg_cluster (connection pool)   │ │
│  │  - observer_manager (shared)      │ │
│  └───────────────────────────────────┘ │
└─────────────────────────────────────────┘
         │
         ├──────────────────────────────────┐
         │                                  │
         ▼                                  ▼
┌──────────────────┐            ┌──────────────────┐
│  Game Session 1  │            │  Game Session 2  │
│  - 3 players     │            │  - 5 players     │
│  - question #2   │            │  - question #1   │
└──────────────────┘            └──────────────────┘
         │                                  │
         ▼                                  ▼
┌─────────────────────────────────────────────────┐
│  PostgreSQL (Connection Pool)                   │
│  - Изоляция на уровне БД                        │
│  - Каждая сессия = отдельные строки             │
└─────────────────────────────────────────────────┘
```

**Ключевые моменты**:
- Один GameService обслуживает все сессии
- Connection pooling обеспечивает параллелизм
- Изоляция на уровне данных (разные `game_session_id`)
- Observers получают события от всех сессий

**Производительность**:
- ✅ Асинхронная обработка (userver)
- ✅ Connection pooling
- ⚠️ Нет кэширования активных игр
- ⚠️ Каждый запрос идет в БД

---

### Сценарий 5: Обработка ошибок

#### 5.1 Ошибка валидации ответа

```
Player → POST /games/{id}/answers
         { "player_id": "invalid-uuid" }
         
Handler → Валидация UUID
          ❌ Invalid format
          
Response ← 400 Bad Request
           { "error": "Invalid UUID format" }
```

#### 5.2 Игрок отвечает дважды

```
Player → POST /games/{id}/answers (1st time)
         ✅ Success
         
Player → POST /games/{id}/answers (2nd time)
         
GameService → GetAnswersCountForQuestion()
              ❌ Player already answered
              
Response ← 400 Bad Request
           { "error": "Player already answered this question" }
```

#### 5.3 База данных недоступна

```
Player → POST /games/{id}/answers
         
GameService → NStorage::SubmitPlayerAnswer()
              
Storage → PostgreSQL
          ❌ Connection timeout
          
GameService ← Exception
              
Handler ← std::exception
          LOG_ERROR()
          
Response ← 500 Internal Server Error
           { "error": "Failed to submit answer" }
```

---

## Диаграммы последовательности

### Полный игровой цикл

```
Admin    Player1   Player2   Handler   GameService   Storage   PostgreSQL   Observers
  │         │         │         │           │           │          │            │
  │ POST /games       │         │           │           │          │            │
  ├─────────────────────────────>           │           │          │            │
  │         │         │         │ CreateGameSession()   │          │            │
  │         │         │         ├──────────>│           │          │            │
  │         │         │         │           │ INSERT    │          │            │
  │         │         │         │           ├──────────>│          │            │
  │         │         │         │           │           │ INSERT   │            │
  │         │         │         │           │           ├─────────>│            │
  │         │         │         │           │           │<─────────┤            │
  │         │         │         │           │<──────────┤          │            │
  │         │         │         │           │ NotifyObservers()    │            │
  │         │         │         │           ├─────────────────────────────────>│
  │         │         │         │<──────────┤           │          │            │
  │<────────────────────────────┤           │           │          │            │
  │ {game_id}         │         │           │           │          │            │
  │         │         │         │           │           │          │            │
  │         │ POST /games/{id}/players      │           │          │            │
  │         ├─────────────────────────────>│           │          │            │
  │         │         │         │ AddPlayer()           │          │            │
  │         │         │         ├──────────>│           │          │            │
  │         │         │         │           │ INSERT    │          │            │
  │         │         │         │           ├──────────>│          │            │
  │         │         │         │           │           │ INSERT   │            │
  │         │         │         │           │           ├─────────>│            │
  │         │         │         │           │ NotifyObservers()    │            │
  │         │         │         │           ├─────────────────────────────────>│
  │         │<────────────────────────────┤           │          │            │
  │         │ {player_id}       │         │           │          │            │
  │         │         │         │           │           │          │            │
  │ POST /games/{id}/start      │           │           │          │            │
  ├─────────────────────────────>           │           │          │            │
  │         │         │         │ StartGame()           │          │            │
  │         │         │         ├──────────>│           │          │            │
  │         │         │         │           │ UPDATE    │          │            │
  │         │         │         │           ├──────────>│          │            │
  │         │         │         │           │ NotifyObservers()    │            │
  │         │         │         │           ├─────────────────────────────────>│
  │         │         │         │           │           │          │      ┌─────┴─────┐
  │         │         │         │           │           │          │      │ Push to   │
  │         │         │         │           │           │          │      │ Players   │
  │         │         │         │           │           │          │      └─────┬─────┘
  │         │<────────────────────────────────────────────────────────────────┤
  │         │ "Game started"    │           │           │          │            │
  │         │         │<────────────────────────────────────────────────────────┤
  │         │         │ "Game started"      │           │          │            │
  │         │         │         │           │           │          │            │
  │         │ POST /games/{id}/answers      │           │          │            │
  │         ├─────────────────────────────>│           │          │            │
  │         │         │         │ SubmitAnswer()        │          │            │
  │         │         │         ├──────────>│           │          │            │
  │         │         │         │           │ Validate  │          │            │
  │         │         │         │           │ INSERT    │          │            │
  │         │         │         │           ├──────────>│          │            │
  │         │         │         │           │ UPDATE score         │            │
  │         │         │         │           ├──────────>│          │            │
  │         │         │         │           │ NotifyObservers()    │            │
  │         │         │         │           ├─────────────────────────────────>│
  │         │<────────────────────────────┤           │          │            │
  │         │ {is_correct: true}          │           │          │            │
  │         │         │         │           │           │          │            │
  │         │         │ POST /games/{id}/answers      │          │            │
  │         │         ├─────────────────────────────>│          │            │
  │         │         │         │ SubmitAnswer()      │          │            │
  │         │         │         ├──────────>│         │          │            │
  │         │         │         │           │ (same as above)     │            │
  │         │         │         │           │ CheckAllAnswered()  │            │
  │         │         │         │           │ AdvanceToNext()     │            │
  │         │         │         │           ├──────────>│         │            │
  │         │         │         │           │ UPDATE current_question_index    │
  │         │         │         │           │           ├─────────>│            │
  │         │         │         │           │ NotifyObservers()   │            │
  │         │         │         │           ├─────────────────────────────────>│
  │         │<────────────────────────────────────────────────────────────────┤
  │         │ "Next question"   │           │           │          │            │
  │         │         │<────────────────────────────────────────────────────────┤
  │         │         │ "Next question"     │           │          │            │
```

---

## Состояния и переходы

### Состояния игровой сессии

```
┌──────────┐
│ waiting  │ ◄─── Начальное состояние
└────┬─────┘
     │ POST /games/{id}/start
     │ (минимум 1 игрок)
     ▼
┌──────────┐
│  active  │ ◄─── Игра идет
└────┬─────┘
     │ Все игроки ответили на последний вопрос
     │
     ▼
┌──────────┐
│ finished │ ◄─── Финальное состояние
└──────────┘
```

**Допустимые операции по состояниям**:

| Операция | waiting | active | finished |
|----------|---------|--------|----------|
| Add Player | ✅ | ❌ | ❌ |
| Start Game | ✅ | ❌ | ❌ |
| Submit Answer | ❌ | ✅ | ❌ |
| Get State | ✅ | ✅ | ✅ |
| Get Results | ❌ | ❌ | ✅ |

---

### Жизненный цикл вопроса

```
┌─────────────────┐
│ Question        │
│ Presented       │
└────────┬────────┘
         │
         │ Players submit answers
         │
         ▼
┌─────────────────┐
│ Collecting      │
│ Answers         │
└────────┬────────┘
         │
         │ All players answered
         │
         ▼
┌─────────────────┐
│ Question        │
│ Completed       │
└────────┬────────┘
         │
         ├─── If not last question ───┐
         │                            │
         ▼                            ▼
┌─────────────────┐          ┌─────────────────┐
│ Advance to      │          │ Game            │
│ Next Question   │          │ Finished        │
└─────────────────┘          └─────────────────┘
```

---

## Обработка событий

### Типы событий и их обработчики

#### 1. GameSessionCreatedEvent
```cpp
struct GameSessionCreatedEvent {
    boost::uuids::uuid game_session_id;
    boost::uuids::uuid pack_id;
};
```

**Обработчики**:
- `LoggingObserver`: Логирует создание сессии
- `StatisticsObserver`: Увеличивает счетчик активных игр

#### 2. PlayerAddedEvent
```cpp
struct PlayerAddedEvent {
    boost::uuids::uuid game_session_id;
    Models::Player player;
};
```

**Обработчики**:
- `LoggingObserver`: Логирует присоединение игрока
- `NotificationObserver`: Уведомляет других игроков

#### 3. AnswerSubmittedEvent
```cpp
struct AnswerSubmittedEvent {
    boost::uuids::uuid game_session_id;
    boost::uuids::uuid player_id;
    boost::uuids::uuid question_id;
    boost::uuids::uuid variant_id;
    bool is_correct;
    std::string player_name;
};
```

**Обработчики**:
- `LoggingObserver`: Логирует ответ
- `NotificationObserver`: Уведомляет других игроков о прогрессе
- `StatisticsObserver`: Обновляет статистику правильных/неправильных ответов

#### 4. GameFinishedEvent
```cpp
struct GameFinishedEvent {
    boost::uuids::uuid game_session_id;
    int total_questions;
    int total_players;
};
```

**Обработчики**:
- `LoggingObserver`: Логирует завершение игры
- `NotificationObserver`: Отправляет финальные результаты всем игрокам
- `StatisticsObserver`: Сохраняет статистику игры

---

### Добавление custom observer

```cpp
class MyCustomObserver : public IGameObserver {
public:
    void OnEvent(const GameEvent& event) override {
        std::visit([this](const auto& e) {
            HandleEvent(e);
        }, event);
    }
    
    bool ShouldHandleEvent(GameEventType type) const override {
        // Обрабатываем только определенные события
        return type == GameEventType::kAnswerSubmitted ||
               type == GameEventType::kGameFinished;
    }
    
private:
    void HandleEvent(const AnswerSubmittedEvent& event) {
        // Отправка в внешний сервис аналитики
        analytics_service_->TrackAnswer(event);
    }
    
    void HandleEvent(const GameFinishedEvent& event) {
        // Отправка финальной статистики
        analytics_service_->TrackGameFinished(event);
    }
};

// Регистрация observer
game_service.AddObserver(std::make_shared<MyCustomObserver>());
```

---

## Оптимизация потоков данных

### Текущие узкие места

1. **Каждый запрос идет в БД**
   - `GetGameState()` делает 3-4 запроса
   - Нет кэширования

2. **Нет batch операций**
   - Каждый ответ = отдельная транзакция
   - Можно батчить ответы от разных игроков

3. **Observers вызываются синхронно**
   - Блокируют основной поток
   - Могут замедлить ответ клиенту

### Рекомендации по оптимизации

#### 1. Кэширование активных игр
```cpp
class GameServiceComponent {
    // In-memory cache
    std::unordered_map<uuid, CachedGameState> active_games_;
    std::shared_mutex cache_mutex_;
    
    CachedGameState GetGameState(uuid game_id) {
        {
            std::shared_lock lock(cache_mutex_);
            if (auto it = active_games_.find(game_id); 
                it != active_games_.end()) {
                return it->second;  // Cache hit
            }
        }
        
        // Cache miss - load from DB
        auto state = LoadFromDatabase(game_id);
        
        {
            std::unique_lock lock(cache_mutex_);
            active_games_[game_id] = state;
        }
        
        return state;
    }
};
```

#### 2. Асинхронные observers
```cpp
void GameService::NotifyObservers(const GameEvent& event) {
    // Не блокируем основной поток
    userver::engine::AsyncNoSpan([this, event]() {
        observer_manager_.NotifyObservers(event);
    }).Detach();
}
```

#### 3. Batch операции
```cpp
// Вместо:
for (auto& answer : answers) {
    SubmitAnswer(answer);  // N транзакций
}

// Использовать:
SubmitAnswersBatch(answers);  // 1 транзакция
```

---

## Заключение

Система спроектирована с учетом:
- ✅ Четкого разделения ответственности
- ✅ Расширяемости через Observer pattern
- ✅ Изоляции данных между сессиями
- ⚠️ Но требует оптимизации для высоких нагрузок

Следующие шаги см. в [`REFACTORING_SUMMARY.md`](../REFACTORING_SUMMARY.md).