# Архитектура команд и валидации

Гранулярная архитектура с атомарными командами, валидацией и композицией.

---

## Концепция

### Проблемы текущего подхода

```cpp
// Сейчас: Монолитные методы
class GameService {
    SubmitAnswer(...);  // Делает ВСЁ: валидация + сохранение + переход
};
```

**Проблемы**:
- Невозможно переиспользовать части логики
- Сложно комбинировать действия
- Валидация смешана с бизнес-логикой
- Фронтенд должен знать о сложной логике

### Новый подход: Атомарные команды

```cpp
// Атомарные команды
SubmitAnswerCommand         // Только сохранить ответ
ValidateAnswerCommand       // Только проверить правильность
UpdateScoreCommand          // Только обновить счет
AdvanceQuestionCommand      // Только перейти к следующему
CheckAllAnsweredCommand     // Только проверить, все ли ответили
```

**Преимущества**:
- Каждая команда делает одно дело
- Легко комбинировать
- Переиспользование
- Фронтенд выбирает, что нужно

---

## Архитектура команд

### 1. Базовый интерфейс команды

```cpp
// src/logic/game/commands/command_interface.hpp
namespace game_userver::logic::game::commands {

// Результат выполнения команды
struct CommandResult {
    bool success;
    std::string message;
    nlohmann::json data;
    
    static CommandResult Success(const nlohmann::json& data = {}) {
        return {true, "Success", data};
    }
    
    static CommandResult Error(const std::string& message) {
        return {false, message, {}};
    }
};

// Контекст выполнения команды
struct CommandContext {
    boost::uuids::uuid game_id;
    boost::uuids::uuid player_id;
    ClusterPtr pg_cluster;
    std::shared_ptr<IGame> game;
    nlohmann::json params;
};

// Базовый интерфейс команды
class ICommand {
public:
    virtual ~ICommand() = default;
    
    // Валидация перед выполнением
    virtual auto Validate(const CommandContext& ctx) const 
        -> CommandResult = 0;
    
    // Выполнение команды
    virtual auto Execute(const CommandContext& ctx) 
        -> CommandResult = 0;
    
    // Откат команды (для транзакций)
    virtual auto Rollback(const CommandContext& ctx) 
        -> CommandResult {
        return CommandResult::Success();
    }
    
    // Метаданные
    virtual auto GetCommandName() const -> std::string = 0;
    virtual auto RequiresTransaction() const -> bool { return false; }
};

} // namespace
```

---

### 2. Базовая реализация с валидацией

```cpp
// src/logic/game/commands/base_command.hpp
namespace game_userver::logic::game::commands {

// Базовая реализация с общей валидацией
class BaseCommand : public ICommand {
public:
    auto Validate(const CommandContext& ctx) const 
        -> CommandResult override {
        
        // Общая валидация
        if (!ctx.game) {
            return CommandResult::Error("Game not found");
        }
        
        if (!ValidateGameState(ctx)) {
            return CommandResult::Error("Invalid game state");
        }
        
        if (!ValidatePlayer(ctx)) {
            return CommandResult::Error("Invalid player");
        }
        
        // Специфичная валидация
        return ValidateImpl(ctx);
    }
    
protected:
    // Хуки для переопределения
    virtual auto ValidateImpl(const CommandContext& ctx) const 
        -> CommandResult {
        return CommandResult::Success();
    }
    
    virtual auto ValidateGameState(const CommandContext& ctx) const 
        -> bool {
        return true;  // Переопределить в наследниках
    }
    
    virtual auto ValidatePlayer(const CommandContext& ctx) const 
        -> bool {
        // Проверка, что игрок существует и в этой игре
        auto players = ctx.game->GetPlayers();
        return std::any_of(players.begin(), players.end(),
            [&](const auto& p) { return p.id == ctx.player_id; });
    }
};

} // namespace
```

---

### 3. Конкретные команды для квиза

#### SubmitAnswerCommand

```cpp
// src/logic/game/commands/quiz/submit_answer_command.hpp
namespace game_userver::logic::game::commands::quiz {

class SubmitAnswerCommand : public BaseCommand {
public:
    auto GetCommandName() const -> std::string override {
        return "submit_answer";
    }
    
    auto RequiresTransaction() const -> bool override {
        return true;  // Нужна транзакция
    }
    
protected:
    auto ValidateGameState(const CommandContext& ctx) const 
        -> bool override {
        // Игра должна быть активна
        return ctx.game->GetState() == GameState::kActive;
    }
    
    auto ValidateImpl(const CommandContext& ctx) const 
        -> CommandResult override {
        
        // Проверка наличия variant_id
        if (!ctx.params.contains("variant_id")) {
            return CommandResult::Error("variant_id is required");
        }
        
        auto variant_id = Utils::StringToUuid(
            ctx.params["variant_id"].get<std::string>()
        );
        
        // Проверка, что игрок еще не ответил
        if (HasPlayerAnswered(ctx, variant_id)) {
            return CommandResult::Error("Player already answered");
        }
        
        // Проверка, что вариант принадлежит текущему вопросу
        if (!IsVariantForCurrentQuestion(ctx, variant_id)) {
            return CommandResult::Error("Invalid variant for current question");
        }
        
        return CommandResult::Success();
    }
    
    auto Execute(const CommandContext& ctx) 
        -> CommandResult override {
        
        auto variant_id = Utils::StringToUuid(
            ctx.params["variant_id"].get<std::string>()
        );
        
        // Получаем текущий вопрос
        auto quiz_game = std::dynamic_pointer_cast<QuizGame>(ctx.game);
        auto question = quiz_game->GetCurrentQuestion();
        
        // Сохраняем ответ в БД
        auto result = ctx.pg_cluster->Execute(
            "INSERT INTO quiz.player_answers "
            "(player_id, question_id, variant_id, is_correct) "
            "VALUES ($1, $2, $3, "
            "  (SELECT is_correct FROM quiz.variants WHERE id = $3)"
            ") "
            "RETURNING is_correct",
            ctx.player_id, question.id, variant_id
        );
        
        bool is_correct = result.AsSingleRow<bool>();
        
        return CommandResult::Success({
            {"is_correct", is_correct},
            {"variant_id", boost::uuids::to_string(variant_id)}
        });
    }
    
private:
    bool HasPlayerAnswered(
        const CommandContext& ctx,
        const boost::uuids::uuid& variant_id
    ) const {
        auto quiz_game = std::dynamic_pointer_cast<QuizGame>(ctx.game);
        auto question = quiz_game->GetCurrentQuestion();
        
        auto result = ctx.pg_cluster->Execute(
            "SELECT COUNT(*) FROM quiz.player_answers "
            "WHERE player_id = $1 AND question_id = $2",
            ctx.player_id, question.id
        );
        
        return result.AsSingleRow<int>() > 0;
    }
    
    bool IsVariantForCurrentQuestion(
        const CommandContext& ctx,
        const boost::uuids::uuid& variant_id
    ) const {
        auto quiz_game = std::dynamic_pointer_cast<QuizGame>(ctx.game);
        auto question = quiz_game->GetCurrentQuestion();
        
        auto result = ctx.pg_cluster->Execute(
            "SELECT COUNT(*) FROM quiz.variants "
            "WHERE id = $1 AND question_id = $2",
            variant_id, question.id
        );
        
        return result.AsSingleRow<int>() > 0;
    }
};

} // namespace
```

#### UpdateScoreCommand

```cpp
// src/logic/game/commands/quiz/update_score_command.hpp
class UpdateScoreCommand : public BaseCommand {
public:
    auto GetCommandName() const -> std::string override {
        return "update_score";
    }
    
    auto RequiresTransaction() const -> bool override {
        return true;
    }
    
protected:
    auto ValidateImpl(const CommandContext& ctx) const 
        -> CommandResult override {
        
        if (!ctx.params.contains("points")) {
            return CommandResult::Error("points is required");
        }
        
        int points = ctx.params["points"].get<int>();
        if (points < 0) {
            return CommandResult::Error("points must be non-negative");
        }
        
        return CommandResult::Success();
    }
    
    auto Execute(const CommandContext& ctx) 
        -> CommandResult override {
        
        int points = ctx.params["points"].get<int>();
        
        auto result = ctx.pg_cluster->Execute(
            "UPDATE quiz.players "
            "SET score = score + $1 "
            "WHERE id = $2 "
            "RETURNING score",
            points, ctx.player_id
        );
        
        int new_score = result.AsSingleRow<int>();
        
        return CommandResult::Success({
            {"new_score", new_score},
            {"points_added", points}
        });
    }
};
```

#### AdvanceQuestionCommand

```cpp
// src/logic/game/commands/quiz/advance_question_command.hpp
class AdvanceQuestionCommand : public BaseCommand {
public:
    auto GetCommandName() const -> std::string override {
        return "advance_question";
    }
    
    auto RequiresTransaction() const -> bool override {
        return true;
    }
    
protected:
    auto ValidateGameState(const CommandContext& ctx) const 
        -> bool override {
        return ctx.game->GetState() == GameState::kActive;
    }
    
    auto ValidateImpl(const CommandContext& ctx) const 
        -> CommandResult override {
        
        auto quiz_game = std::dynamic_pointer_cast<QuizGame>(ctx.game);
        
        // Проверка, что есть следующий вопрос
        if (quiz_game->IsLastQuestion()) {
            return CommandResult::Error("Already at last question");
        }
        
        return CommandResult::Success();
    }
    
    auto Execute(const CommandContext& ctx) 
        -> CommandResult override {
        
        auto quiz_game = std::dynamic_pointer_cast<QuizGame>(ctx.game);
        int next_index = quiz_game->GetCurrentQuestionIndex() + 1;
        
        // Обновляем индекс в БД
        ctx.pg_cluster->Execute(
            "UPDATE quiz.game_sessions "
            "SET current_question_index = $1, "
            "    question_started_at = NOW() "
            "WHERE id = $2",
            next_index, ctx.game_id
        );
        
        // Обновляем в памяти
        quiz_game->SetCurrentQuestionIndex(next_index);
        
        return CommandResult::Success({
            {"new_question_index", next_index},
            {"question", quiz_game->GetCurrentQuestion().ToJson()}
        });
    }
};
```

#### CheckAllAnsweredCommand

```cpp
// src/logic/game/commands/quiz/check_all_answered_command.hpp
class CheckAllAnsweredCommand : public BaseCommand {
public:
    auto GetCommandName() const -> std::string override {
        return "check_all_answered";
    }
    
protected:
    auto Execute(const CommandContext& ctx) 
        -> CommandResult override {
        
        auto quiz_game = std::dynamic_pointer_cast<QuizGame>(ctx.game);
        auto question = quiz_game->GetCurrentQuestion();
        
        // Получаем количество игроков
        auto players = ctx.game->GetPlayers();
        int total_players = players.size();
        
        // Получаем количество ответов
        auto result = ctx.pg_cluster->Execute(
            "SELECT COUNT(DISTINCT player_id) "
            "FROM quiz.player_answers "
            "WHERE question_id = $1",
            question.id
        );
        
        int answered_count = result.AsSingleRow<int>();
        bool all_answered = (answered_count >= total_players);
        
        return CommandResult::Success({
            {"all_answered", all_answered},
            {"answered_count", answered_count},
            {"total_players", total_players}
        });
    }
};
```

---

### 4. Command Registry

```cpp
// src/logic/game/commands/command_registry.hpp
namespace game_userver::logic::game::commands {

class CommandRegistry {
public:
    using CommandCreator = std::function<std::unique_ptr<ICommand>()>;
    
    // Регистрация команды
    void RegisterCommand(
        const std::string& name,
        CommandCreator creator
    ) {
        commands_[name] = creator;
    }
    
    // Создание команды по имени
    auto CreateCommand(const std::string& name) 
        -> std::unique_ptr<ICommand> {
        
        auto it = commands_.find(name);
        if (it == commands_.end()) {
            return nullptr;
        }
        
        return it->second();
    }
    
    // Получение списка доступных команд
    auto GetAvailableCommands() const 
        -> std::vector<std::string> {
        
        std::vector<std::string> names;
        for (const auto& [name, _] : commands_) {
            names.push_back(name);
        }
        return names;
    }
    
private:
    std::unordered_map<std::string, CommandCreator> commands_;
};

// Глобальный реестр для квиза
inline CommandRegistry& GetQuizCommandRegistry() {
    static CommandRegistry registry;
    static bool initialized = false;
    
    if (!initialized) {
        // Регистрация всех команд квиза
        registry.RegisterCommand("submit_answer", 
            []() { return std::make_unique<SubmitAnswerCommand>(); });
        
        registry.RegisterCommand("update_score",
            []() { return std::make_unique<UpdateScoreCommand>(); });
        
        registry.RegisterCommand("advance_question",
            []() { return std::make_unique<AdvanceQuestionCommand>(); });
        
        registry.RegisterCommand("check_all_answered",
            []() { return std::make_unique<CheckAllAnsweredCommand>(); });
        
        initialized = true;
    }
    
    return registry;
}

} // namespace
```

---

### 5. Command Executor с транзакциями

```cpp
// src/logic/game/commands/command_executor.hpp
namespace game_userver::logic::game::commands {

class CommandExecutor {
public:
    explicit CommandExecutor(ClusterPtr pg_cluster);
    
    // Выполнение одной команды
    auto ExecuteCommand(
        const std::string& command_name,
        const CommandContext& ctx
    ) -> CommandResult;
    
    // Выполнение цепочки команд (транзакция)
    auto ExecuteCommandChain(
        const std::vector<std::string>& command_names,
        const CommandContext& ctx
    ) -> CommandResult;
    
private:
    ClusterPtr pg_cluster_;
    CommandRegistry& registry_;
};

auto CommandExecutor::ExecuteCommand(
    const std::string& command_name,
    const CommandContext& ctx
) -> CommandResult {
    
    // Создаем команду
    auto command = registry_.CreateCommand(command_name);
    if (!command) {
        return CommandResult::Error("Unknown command: " + command_name);
    }
    
    // Валидация
    auto validation_result = command->Validate(ctx);
    if (!validation_result.success) {
        LOG_WARNING() << "Command validation failed: " 
                     << command_name << " - " 
                     << validation_result.message;
        return validation_result;
    }
    
    // Выполнение
    try {
        if (command->RequiresTransaction()) {
            // В транзакции
            auto trx = pg_cluster_->Begin(
                ClusterHostType::kMaster,
                Transaction::RW
            );
            
            auto result = command->Execute(ctx);
            
            if (result.success) {
                trx.Commit();
            } else {
                trx.Rollback();
            }
            
            return result;
        } else {
            // Без транзакции
            return command->Execute(ctx);
        }
    } catch (const std::exception& e) {
        LOG_ERROR() << "Command execution failed: " 
                   << command_name << " - " << e.what();
        return CommandResult::Error("Execution failed: " + std::string(e.what()));
    }
}

auto CommandExecutor::ExecuteCommandChain(
    const std::vector<std::string>& command_names,
    const CommandContext& ctx
) -> CommandResult {
    
    // Проверяем, нужна ли транзакция
    bool needs_transaction = false;
    std::vector<std::unique_ptr<ICommand>> commands;
    
    for (const auto& name : command_names) {
        auto command = registry_.CreateCommand(name);
        if (!command) {
            return CommandResult::Error("Unknown command: " + name);
        }
        
        if (command->RequiresTransaction()) {
            needs_transaction = true;
        }
        
        commands.push_back(std::move(command));
    }
    
    // Валидация всех команд
    for (const auto& command : commands) {
        auto validation_result = command->Validate(ctx);
        if (!validation_result.success) {
            return validation_result;
        }
    }
    
    // Выполнение
    try {
        if (needs_transaction) {
            auto trx = pg_cluster_->Begin(
                ClusterHostType::kMaster,
                Transaction::RW
            );
            
            nlohmann::json results = nlohmann::json::array();
            
            for (auto& command : commands) {
                auto result = command->Execute(ctx);
                if (!result.success) {
                    trx.Rollback();
                    return result;
                }
                results.push_back(result.data);
            }
            
            trx.Commit();
            
            return CommandResult::Success({{"results", results}});
        } else {
            nlohmann::json results = nlohmann::json::array();
            
            for (auto& command : commands) {
                auto result = command->Execute(ctx);
                if (!result.success) {
                    return result;
                }
                results.push_back(result.data);
            }
            
            return CommandResult::Success({{"results", results}});
        }
    } catch (const std::exception& e) {
        LOG_ERROR() << "Command chain execution failed: " << e.what();
        return CommandResult::Error("Chain execution failed");
    }
}

} // namespace
```

---

### 6. Универсальный Handler

```cpp
// src/handlers/game/execute_command_handler.hpp
class ExecuteCommandHandler : public HttpHandlerBase {
public:
    auto HandleRequestThrow(...) const -> std::string override {
        auto json = userver::formats::json::FromString(request_body);
        
        auto game_id = Utils::StringToUuid(request.GetPathArg("game_id"));
        auto player_id = Utils::StringToUuid(json["player_id"]);
        auto command_name = json["command"].As<std::string>();
        auto params = json["params"];
        
        // Получаем игру
        auto game = game_service_->GetGame(game_id);
        if (!game) {
            return ErrorResponse("Game not found");
        }
        
        // Формируем контекст
        CommandContext ctx{
            .game_id = game_id,
            .player_id = player_id,
            .pg_cluster = pg_cluster_,
            .game = game,
            .params = params
        };
        
        // Выполняем команду
        auto result = command_executor_->ExecuteCommand(command_name, ctx);
        
        return FormatResponse(result);
    }
};

// Для цепочки команд
class ExecuteCommandChainHandler : public HttpHandlerBase {
public:
    auto HandleRequestThrow(...) const -> std::string override {
        auto json = userver::formats::json::FromString(request_body);
        
        auto game_id = Utils::StringToUuid(request.GetPathArg("game_id"));
        auto player_id = Utils::StringToUuid(json["player_id"]);
        auto commands = json["commands"].As<std::vector<std::string>>();
        auto params = json["params"];
        
        CommandContext ctx{...};
        
        // Выполняем цепочку
        auto result = command_executor_->ExecuteCommandChain(commands, ctx);
        
        return FormatResponse(result);
    }
};
```

---

### 7. API для фронтенда

#### Одна команда

```http
POST /games/{game_id}/commands
{
  "player_id": "uuid",
  "command": "submit_answer",
  "params": {
    "variant_id": "uuid"
  }
}

Response:
{
  "success": true,
  "message": "Success",
  "data": {
    "is_correct": true,
    "variant_id": "uuid"
  }
}
```

#### Цепочка команд (композиция)

```http
POST /games/{game_id}/commands/chain
{
  "player_id": "uuid",
  "commands": [
    "submit_answer",
    "update_score",
    "check_all_answered"
  ],
  "params": {
    "variant_id": "uuid",
    "points": 1
  }
}

Response:
{
  "success": true,
  "data": {
    "results": [
      {"is_correct": true},
      {"new_score": 5},
      {"all_answered": false, "answered_count": 2, "total_players": 3}
    ]
  }
}
```

#### Автопереход (композиция на фронтенде)

```http
# Фронтенд отправляет цепочку
POST /games/{game_id}/commands/chain
{
  "player_id": "uuid",
  "commands": [
    "submit_answer",
    "update_score",
    "check_all_answered",
    "advance_question"  # Условно выполнится, если все ответили
  ],
  "params": {...}
}
```

---

## Преимущества архитектуры

### 1. Гранулярность ✅
- Каждая команда = одно действие
- Легко комбинировать
- Переиспользование

### 2. Валидация на всех уровнях ✅
- Middleware (HTTP уровень)
- Command.Validate() (бизнес-логика)
- БД constraints (данные)

### 3. Расширяемость ✅
- Новая команда = один класс
- Регистрация в реестре
- Фронтенд выбирает, что использовать

### 4. Адаптеры для фронтендов ✅
```
Telegram Bot Adapter → Композиция команд → Backend
Web Adapter → Другая композиция → Backend
Mobile Adapter → Своя композиция → Backend
```

### 5. Транзакции ✅
- Автоматически для команд с `RequiresTransaction()`
- Цепочка команд = одна транзакция
- Откат при ошибке

---

## Пример: Telegram Bot Adapter

```python
# telegram_bot/adapters/quiz_adapter.py
class QuizGameAdapter:
    def __init__(self, backend_url):
        self.backend_url = backend_url
    
    def handle_answer_button(self, game_id, player_id, variant_id):
        """Обработка нажатия кнопки с ответом"""
        
        # Композиция команд для автоперехода
        response = requests.post(
            f"{self.backend_url}/games/{game_id}/commands/chain",
            json={
                "player_id": player_id,
                "commands": [
                    "submit_answer",      # Сохранить ответ
                    "update_score",       # Обновить счет если правильно
                    "check_all_answered"  # Проверить, все ли ответили
                ],
                "params": {
                    "variant_id": variant_id,
                    "points": 1
                }
            }
        )
        
        data = response.json()
        
        # Если все ответили - переходим к следующему вопросу
        if data['data']['results'][2]['all_answered']:
            self.advance_question(game_id, player_id)
    
    def advance_question(self, game_id, player_id):
        """Переход к следующему вопросу"""
        requests.post(
            f"{self.backend_url}/games/{game_id}/commands",
            json={
                "player_id": player_id,
                "command": "advance_question",
                "params": {}
            }
        )
```

---

## Заключение

Новая архитектура обеспечивает:
- ✅ Атомарные команды
- ✅ Композиция на уровне фронтенда
- ✅ Многоуровневая валидация
- ✅ Транзакции
- ✅ Адаптеры для разных фронтендов
- ✅ Расширяемость без изменения бэкенда

Фронтенд-разработчик может:
- Выбирать нужные команды
- Комбинировать их
- Создавать свою логику
- Не зависеть от бэкенд-разработчика