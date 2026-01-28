# Архитектура игровой платформы

Предложение по рефакторингу текущей системы в расширяемую игровую платформу.

---

## Концепция

Вместо жесткой привязки к квизам, создать **абстрактную игровую платформу**, где:
- Квиз - это один из типов игр
- Легко добавлять новые типы игр (шахматы, карточные игры, etc.)
- Фронтенд-агностичность (Telegram Bot, Web, Mobile)
- Переиспользование общей логики (старт, финиш, игроки, observers)

---

## Текущая проблема

```cpp
// Сейчас: Жесткая привязка к квизам
class GameService {
    CreateGameSession(pack_id);  // Только для квизов!
    SubmitAnswer(variant_id);    // Только для квизов!
};
```

**Проблемы**:
- Невозможно добавить другие типы игр
- Логика квиза смешана с общей логикой игры
- Нет абстракции "игра"

---

## Предлагаемая архитектура

### 1. Абстрактный интерфейс игры

```cpp
// src/logic/game/base/game_interface.hpp
namespace game_userver::logic::game {

// Базовый интерфейс любой игры
class IGame {
public:
    virtual ~IGame() = default;
    
    // Жизненный цикл игры (общее для всех игр)
    virtual auto Start() -> GameResult = 0;
    virtual auto Finish() -> GameResult = 0;
    virtual auto GetState() const -> GameState = 0;
    
    // Управление игроками (общее)
    virtual auto AddPlayer(const std::string& player_name) 
        -> std::optional<Models::Player> = 0;
    virtual auto RemovePlayer(const boost::uuids::uuid& player_id) 
        -> bool = 0;
    virtual auto GetPlayers() const 
        -> std::vector<Models::Player> = 0;
    
    // Игровая логика (специфично для каждой игры)
    virtual auto ProcessPlayerAction(
        const boost::uuids::uuid& player_id,
        const PlayerAction& action
    ) -> ActionResult = 0;
    
    // Метаданные
    virtual auto GetGameType() const -> GameType = 0;
    virtual auto GetGameId() const -> boost::uuids::uuid = 0;
    
    // Observer support (общее)
    virtual void AddObserver(std::shared_ptr<IGameObserver> observer) = 0;
    virtual void RemoveObserver(std::shared_ptr<IGameObserver> observer) = 0;
    
protected:
    // Хуки для наследников
    virtual void OnGameStarted() {}
    virtual void OnGameFinished() {}
    virtual void OnPlayerAdded(const Models::Player& player) {}
    virtual void OnPlayerRemoved(const boost::uuids::uuid& player_id) {}
};

// Типы игр
enum class GameType {
    kQuiz,
    kChess,
    kPoker,
    kTicTacToe,
    // ... другие типы
};

// Универсальное действие игрока
struct PlayerAction {
    boost::uuids::uuid player_id;
    std::string action_type;  // "submit_answer", "make_move", etc.
    nlohmann::json data;      // Гибкие данные
};

struct ActionResult {
    bool success;
    std::string message;
    nlohmann::json data;  // Результат действия
};

} // namespace
```

---

### 2. Базовая реализация с общей логикой

```cpp
// src/logic/game/base/base_game.hpp
namespace game_userver::logic::game {

// Базовая реализация с общей логикой
class BaseGame : public IGame {
public:
    explicit BaseGame(
        const boost::uuids::uuid& game_id,
        ClusterPtr pg_cluster
    );
    
    virtual ~BaseGame() = default;
    
    // Реализация общих методов
    auto Start() -> GameResult override;
    auto Finish() -> GameResult override;
    auto GetState() const -> GameState override;
    
    auto AddPlayer(const std::string& player_name) 
        -> std::optional<Models::Player> override;
    auto RemovePlayer(const boost::uuids::uuid& player_id) 
        -> bool override;
    auto GetPlayers() const 
        -> std::vector<Models::Player> override;
    
    auto GetGameId() const -> boost::uuids::uuid override {
        return game_id_;
    }
    
    // Observer support
    void AddObserver(std::shared_ptr<IGameObserver> observer) override;
    void RemoveObserver(std::shared_ptr<IGameObserver> observer) override;
    
protected:
    // Шаблонный метод для старта
    auto Start() -> GameResult {
        if (state_ != GameState::kWaiting) {
            return GameResult::Error("Game already started");
        }
        
        // Проверка минимального количества игроков
        if (players_.size() < GetMinPlayers()) {
            return GameResult::Error("Not enough players");
        }
        
        // Вызов хука наследника
        auto result = OnStartImpl();
        if (!result.success) {
            return result;
        }
        
        // Общая логика
        state_ = GameState::kActive;
        started_at_ = std::chrono::system_clock::now();
        
        // Уведомление observers
        NotifyObservers(GameStartedEvent{game_id_, players_.size()});
        
        // Хук после старта
        OnGameStarted();
        
        return GameResult::Success();
    }
    
    // Шаблонный метод для финиша
    auto Finish() -> GameResult {
        if (state_ != GameState::kActive) {
            return GameResult::Error("Game not active");
        }
        
        // Вызов хука наследника
        auto result = OnFinishImpl();
        if (!result.success) {
            return result;
        }
        
        // Общая логика
        state_ = GameState::kFinished;
        finished_at_ = std::chrono::system_clock::now();
        
        // Уведомление observers
        NotifyObservers(GameFinishedEvent{game_id_, players_.size()});
        
        // Хук после финиша
        OnGameFinished();
        
        return GameResult::Success();
    }
    
    // Хуки для переопределения в наследниках
    virtual auto OnStartImpl() -> GameResult { 
        return GameResult::Success(); 
    }
    
    virtual auto OnFinishImpl() -> GameResult { 
        return GameResult::Success(); 
    }
    
    virtual auto GetMinPlayers() const -> size_t { 
        return 1; 
    }
    
    virtual auto GetMaxPlayers() const -> size_t { 
        return 100; 
    }
    
    // Утилиты для наследников
    void NotifyObservers(const GameEvent& event);
    auto GetDatabase() -> ClusterPtr { return pg_cluster_; }
    
private:
    boost::uuids::uuid game_id_;
    GameState state_;
    std::vector<Models::Player> players_;
    ClusterPtr pg_cluster_;
    GameObserverManager observer_manager_;
    
    std::chrono::system_clock::time_point created_at_;
    std::chrono::system_clock::time_point started_at_;
    std::chrono::system_clock::time_point finished_at_;
};

} // namespace
```

---

### 3. Конкретная реализация: QuizGame

```cpp
// src/logic/game/types/quiz_game.hpp
namespace game_userver::logic::game {

class QuizGame : public BaseGame {
public:
    QuizGame(
        const boost::uuids::uuid& game_id,
        const boost::uuids::uuid& pack_id,
        ClusterPtr pg_cluster
    );
    
    // Реализация специфичной логики
    auto ProcessPlayerAction(
        const boost::uuids::uuid& player_id,
        const PlayerAction& action
    ) -> ActionResult override;
    
    auto GetGameType() const -> GameType override {
        return GameType::kQuiz;
    }
    
protected:
    // Переопределение хуков
    auto OnStartImpl() -> GameResult override {
        // Загрузка вопросов из БД
        questions_ = LoadQuestions(pack_id_);
        if (questions_.empty()) {
            return GameResult::Error("No questions in pack");
        }
        
        current_question_index_ = 0;
        return GameResult::Success();
    }
    
    auto OnFinishImpl() -> GameResult override {
        // Подсчет финальных результатов
        CalculateFinalScores();
        return GameResult::Success();
    }
    
    auto GetMinPlayers() const -> size_t override {
        return 1;  // Квиз можно играть одному
    }
    
    void OnPlayerAdded(const Models::Player& player) override {
        // Инициализация счета игрока
        player_scores_[player.id] = 0;
    }
    
private:
    // Специфичные для квиза методы
    auto SubmitAnswer(
        const boost::uuids::uuid& player_id,
        const boost::uuids::uuid& variant_id
    ) -> ActionResult;
    
    auto AdvanceToNextQuestion() -> void;
    auto CalculateFinalScores() -> void;
    auto LoadQuestions(const boost::uuids::uuid& pack_id) 
        -> std::vector<QuizQuestion>;
    
    // Специфичные для квиза данные
    boost::uuids::uuid pack_id_;
    std::vector<QuizQuestion> questions_;
    size_t current_question_index_;
    std::unordered_map<boost::uuids::uuid, int> player_scores_;
};

// Реализация ProcessPlayerAction
auto QuizGame::ProcessPlayerAction(
    const boost::uuids::uuid& player_id,
    const PlayerAction& action
) -> ActionResult {
    if (action.action_type == "submit_answer") {
        auto variant_id = action.data["variant_id"].get<std::string>();
        return SubmitAnswer(player_id, Utils::StringToUuid(variant_id));
    }
    
    return ActionResult{
        .success = false,
        .message = "Unknown action type"
    };
}

} // namespace
```

---

### 4. Пример другой игры: TicTacToeGame

```cpp
// src/logic/game/types/tictactoe_game.hpp
namespace game_userver::logic::game {

class TicTacToeGame : public BaseGame {
public:
    TicTacToeGame(
        const boost::uuids::uuid& game_id,
        ClusterPtr pg_cluster
    );
    
    auto ProcessPlayerAction(
        const boost::uuids::uuid& player_id,
        const PlayerAction& action
    ) -> ActionResult override;
    
    auto GetGameType() const -> GameType override {
        return GameType::kTicTacToe;
    }
    
protected:
    auto OnStartImpl() -> GameResult override {
        // Инициализация доски
        board_ = std::vector<std::vector<char>>(3, std::vector<char>(3, ' '));
        current_player_index_ = 0;
        return GameResult::Success();
    }
    
    auto GetMinPlayers() const -> size_t override {
        return 2;  // Крестики-нолики для двоих
    }
    
    auto GetMaxPlayers() const -> size_t override {
        return 2;
    }
    
private:
    auto MakeMove(
        const boost::uuids::uuid& player_id,
        int row, int col
    ) -> ActionResult;
    
    auto CheckWinner() -> std::optional<boost::uuids::uuid>;
    
    std::vector<std::vector<char>> board_;
    size_t current_player_index_;
};

} // namespace
```

---

### 5. Game Factory

```cpp
// src/logic/game/game_factory.hpp
namespace game_userver::logic::game {

class GameFactory {
public:
    explicit GameFactory(ClusterPtr pg_cluster);
    
    // Создание игры по типу
    auto CreateGame(
        GameType type,
        const boost::uuids::uuid& game_id,
        const nlohmann::json& config
    ) -> std::unique_ptr<IGame>;
    
    // Регистрация новых типов игр
    using GameCreator = std::function<
        std::unique_ptr<IGame>(
            const boost::uuids::uuid&,
            const nlohmann::json&,
            ClusterPtr
        )
    >;
    
    void RegisterGameType(GameType type, GameCreator creator);
    
private:
    ClusterPtr pg_cluster_;
    std::unordered_map<GameType, GameCreator> creators_;
};

// Использование
auto factory = GameFactory(pg_cluster);

// Регистрация квиза
factory.RegisterGameType(GameType::kQuiz, 
    [](auto game_id, auto config, auto pg) {
        auto pack_id = Utils::StringToUuid(config["pack_id"]);
        return std::make_unique<QuizGame>(game_id, pack_id, pg);
    }
);

// Регистрация крестиков-ноликов
factory.RegisterGameType(GameType::kTicTacToe,
    [](auto game_id, auto config, auto pg) {
        return std::make_unique<TicTacToeGame>(game_id, pg);
    }
);

// Создание игры
auto game = factory.CreateGame(
    GameType::kQuiz,
    game_id,
    {{"pack_id", "uuid-string"}}
);
```

---

### 6. Универсальный GameService

```cpp
// src/logic/game/game_service.hpp
namespace game_userver::logic::game {

class GameService {
public:
    explicit GameService(
        ClusterPtr pg_cluster,
        std::unique_ptr<GameFactory> factory
    );
    
    // Универсальные методы
    auto CreateGame(
        GameType type,
        const nlohmann::json& config
    ) -> std::optional<boost::uuids::uuid>;
    
    auto GetGame(const boost::uuids::uuid& game_id) 
        -> std::shared_ptr<IGame>;
    
    auto StartGame(const boost::uuids::uuid& game_id) 
        -> GameResult;
    
    auto FinishGame(const boost::uuids::uuid& game_id) 
        -> GameResult;
    
    auto ProcessPlayerAction(
        const boost::uuids::uuid& game_id,
        const boost::uuids::uuid& player_id,
        const PlayerAction& action
    ) -> ActionResult;
    
    // Observer management (глобальные observers для всех игр)
    void AddGlobalObserver(std::shared_ptr<IGameObserver> observer);
    
private:
    ClusterPtr pg_cluster_;
    std::unique_ptr<GameFactory> factory_;
    
    // Активные игры в памяти
    std::unordered_map<
        boost::uuids::uuid, 
        std::shared_ptr<IGame>
    > active_games_;
    
    std::shared_mutex games_mutex_;
    
    // Глобальные observers
    std::vector<std::shared_ptr<IGameObserver>> global_observers_;
};

} // namespace
```

---

### 7. Универсальные Handlers

```cpp
// src/handlers/game/universal_game_handler.hpp
namespace game_userver {

class CreateGameHandler : public HttpHandlerBase {
public:
    auto HandleRequestThrow(...) const -> std::string override {
        auto json = userver::formats::json::FromString(request_body);
        
        // Универсальный подход
        auto game_type_str = json["game_type"].As<std::string>();
        auto game_type = ParseGameType(game_type_str);  // "quiz", "chess", etc.
        
        auto config = json["config"];  // Специфичная конфигурация
        
        auto game_id = game_service_->CreateGame(game_type, config);
        
        return FormatResponse(game_id);
    }
};

class ProcessActionHandler : public HttpHandlerBase {
public:
    auto HandleRequestThrow(...) const -> std::string override {
        auto json = userver::formats::json::FromString(request_body);
        
        auto game_id = Utils::StringToUuid(request.GetPathArg("game_id"));
        auto player_id = Utils::StringToUuid(json["player_id"]);
        
        PlayerAction action{
            .player_id = player_id,
            .action_type = json["action_type"].As<std::string>(),
            .data = json["data"]
        };
        
        auto result = game_service_->ProcessPlayerAction(
            game_id, player_id, action
        );
        
        return FormatResponse(result);
    }
};

} // namespace
```

---

## API для фронтенда

### Универсальный API

```http
# Создание игры (любого типа)
POST /games
{
  "game_type": "quiz",
  "config": {
    "pack_id": "uuid"
  }
}

POST /games
{
  "game_type": "tictactoe",
  "config": {}
}

# Универсальное действие
POST /games/{game_id}/actions
{
  "player_id": "uuid",
  "action_type": "submit_answer",  # или "make_move", "play_card", etc.
  "data": {
    "variant_id": "uuid"  # специфично для квиза
  }
}

POST /games/{game_id}/actions
{
  "player_id": "uuid",
  "action_type": "make_move",
  "data": {
    "row": 1,
    "col": 2
  }
}

# Получение состояния (универсальное)
GET /games/{game_id}/state
{
  "game_id": "uuid",
  "game_type": "quiz",
  "state": "active",
  "players": [...],
  "game_data": {
    // Специфично для типа игры
    "current_question": {...},  // для квиза
    "board": [[...]]            // для крестиков-ноликов
  }
}
```

---

## Преимущества новой архитектуры

### 1. Расширяемость
```cpp
// Добавление новой игры = создание одного класса
class MyNewGame : public BaseGame {
    // Переопределить только специфичную логику
};
```

### 2. Переиспользование кода
- Старт/финиш - общий код в `BaseGame`
- Управление игроками - общий код
- Observers - общий механизм

### 3. Фронтенд-агностичность
- Telegram Bot использует тот же API
- Web-сайт использует тот же API
- Mobile app использует тот же API

### 4. Тестируемость
```cpp
// Легко мокировать IGame
class MockGame : public IGame {
    // Тестовая реализация
};
```

---

## План миграции

### Фаза 1: Создание абстракций (1-2 недели)
1. Создать `IGame` интерфейс
2. Создать `BaseGame` с общей логикой
3. Создать `GameFactory`
4. Обновить `GameService` для работы с `IGame`

### Фаза 2: Рефакторинг квиза (1 неделя)
1. Создать `QuizGame` наследник от `BaseGame`
2. Перенести логику из текущего `GameService`
3. Обновить handlers для работы с новым API

### Фаза 3: Тестирование (1 неделя)
1. Unit-тесты для `BaseGame`
2. Integration-тесты для `QuizGame`
3. Регрессионное тестирование

### Фаза 4: Добавление новых игр (по мере необходимости)
1. Реализовать `TicTacToeGame`
2. Реализовать `ChessGame`
3. etc.

---

## Диаграмма новой архитектуры

```
┌─────────────────────────────────────────┐
│           Frontend Layer                │
│  (Telegram Bot, Web, Mobile)            │
└──────────────┬──────────────────────────┘
               │ Universal API
               ▼
┌─────────────────────────────────────────┐
│         Universal Handlers              │
│  - CreateGameHandler                    │
│  - ProcessActionHandler                 │
│  - GetStateHandler                      │
└──────────────┬──────────────────────────┘
               │
               ▼
┌─────────────────────────────────────────┐
│         GameService                     │
│  - CreateGame(type, config)             │
│  - ProcessPlayerAction()                │
└──────────────┬──────────────────────────┘
               │
               ▼
┌─────────────────────────────────────────┐
│         GameFactory                     │
│  - CreateGame(type) -> IGame            │
└──────────────┬──────────────────────────┘
               │
               ├──────────────┬──────────────┐
               ▼              ▼              ▼
         ┌─────────┐    ┌─────────┐    ┌─────────┐
         │QuizGame │    │ChessGame│    │PokerGame│
         └────┬────┘    └────┬────┘    └────┬────┘
              │              │              │
              └──────────────┴──────────────┘
                           │
                           ▼
                    ┌─────────────┐
                    │  BaseGame   │
                    │  (Template) │
                    └──────┬──────┘
                           │
                           ▼
                    ┌─────────────┐
                    │   IGame     │
                    │ (Interface) │
                    └─────────────┘
```

---

## Заключение

Предложенная архитектура позволит:
- ✅ Легко добавлять новые типы игр
- ✅ Переиспользовать общую логику
- ✅ Поддерживать любые фронтенды
- ✅ Масштабировать платформу
- ✅ Тестировать компоненты изолированно

Это превратит ваш проект из "квиз-системы" в **полноценную игровую платформу**!