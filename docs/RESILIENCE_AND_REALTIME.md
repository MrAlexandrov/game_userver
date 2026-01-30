# Устойчивость к перезагрузкам и Real-time уведомления

Решение трех ключевых проблем:
1. Устойчивость к перезагрузкам (все в БД)
2. Таймер на бэкенде вместо проверки "все ответили"
3. Real-time уведомления фронтенду (Telegram Bot)

---

## Проблема 1: Устойчивость к перезагрузкам

### Текущая проблема

```cpp
// Сейчас: Состояние в памяти
class GameService {
    std::unordered_map<uuid, GameState> active_games_;  // ❌ Потеряется при перезагрузке
    GameObserverManager observer_manager_;              // ❌ Потеряется при перезагрузке
};
```

**Что теряется при перезагрузке**:
- Активные игры в памяти
- Observers
- Таймеры
- WebSocket соединения

### Решение: Все в БД + восстановление состояния

#### 1. Расширенная схема БД

```sql
-- Добавляем поля для таймеров
ALTER TABLE quiz.game_sessions ADD COLUMN IF NOT EXISTS
    question_started_at TIMESTAMP,
    question_timeout_seconds INTEGER DEFAULT 30,
    auto_advance_enabled BOOLEAN DEFAULT TRUE;

-- Таблица для отслеживания активных таймеров
CREATE TABLE IF NOT EXISTS quiz.active_timers (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    game_session_id UUID NOT NULL REFERENCES quiz.game_sessions(id) ON DELETE CASCADE,
    timer_type TEXT NOT NULL,  -- 'question_timeout', 'game_timeout'
    expires_at TIMESTAMP NOT NULL,
    created_at TIMESTAMP NOT NULL DEFAULT NOW(),
    
    INDEX idx_active_timers_expires (expires_at),
    INDEX idx_active_timers_game (game_session_id)
);

-- Таблица для WebSocket подписок (для восстановления)
CREATE TABLE IF NOT EXISTS quiz.game_subscriptions (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    game_session_id UUID NOT NULL REFERENCES quiz.game_sessions(id) ON DELETE CASCADE,
    subscriber_type TEXT NOT NULL,  -- 'telegram_bot', 'web_client'
    subscriber_id TEXT NOT NULL,    -- chat_id для Telegram, session_id для Web
    webhook_url TEXT,               -- URL для отправки уведомлений
    created_at TIMESTAMP NOT NULL DEFAULT NOW(),
    last_ping_at TIMESTAMP,
    
    UNIQUE(game_session_id, subscriber_type, subscriber_id)
);
```

#### 2. Восстановление состояния при старте

```cpp
// src/components/game_service/game_service_component.cpp
GameServiceComponent::GameServiceComponent(
    const ComponentConfig& config,
    const ComponentContext& context
)
    : ComponentBase(config, context) {
    
    auto& pg_component = context.FindComponent<Postgres>(Constants::kDatabaseName);
    auto pg_cluster = pg_component.GetCluster();
    
    game_service_ = std::make_unique<GameService>(pg_cluster);
    
    // Восстановление активных игр из БД
    RestoreActiveGames();
    
    // Восстановление таймеров
    RestoreActiveTimers();
    
    // Восстановление подписок
    RestoreSubscriptions();
    
    LOG_INFO() << "GameServiceComponent initialized and state restored";
}

void GameServiceComponent::RestoreActiveGames() {
    // Загружаем все активные игры из БД
    auto result = pg_cluster_->Execute(
        "SELECT id, pack_id, state, current_question_index, "
        "question_started_at, question_timeout_seconds "
        "FROM quiz.game_sessions "
        "WHERE state = 'active'"
    );
    
    for (const auto& row : result) {
        auto game_id = row["id"].As<boost::uuids::uuid>();
        auto pack_id = row["pack_id"].As<boost::uuids::uuid>();
        
        // Восстанавливаем игру в памяти
        auto game = std::make_shared<QuizGame>(game_id, pack_id, pg_cluster_);
        game->RestoreFromDatabase();  // Загружает игроков, вопросы, etc.
        
        active_games_[game_id] = game;
        
        LOG_INFO() << "Restored active game: " << game_id;
    }
}

void GameServiceComponent::RestoreActiveTimers() {
    auto result = pg_cluster_->Execute(
        "SELECT game_session_id, timer_type, expires_at "
        "FROM quiz.active_timers "
        "WHERE expires_at > NOW()"
    );
    
    for (const auto& row : result) {
        auto game_id = row["game_session_id"].As<boost::uuids::uuid>();
        auto timer_type = row["timer_type"].As<std::string>();
        auto expires_at = row["expires_at"].As<std::chrono::system_clock::time_point>();
        
        // Пересоздаем таймер
        if (timer_type == "question_timeout") {
            ScheduleQuestionTimeout(game_id, expires_at);
        }
        
        LOG_INFO() << "Restored timer for game: " << game_id;
    }
}
```

---

## Проблема 2: Таймер вместо проверки "все ответили"

### Архитектура таймеров

```cpp
// src/logic/game/timer_manager.hpp
namespace game_userver::logic::game {

class TimerManager {
public:
    explicit TimerManager(
        userver::engine::TaskProcessor& task_processor,
        ClusterPtr pg_cluster
    );
    
    // Запуск таймера для вопроса
    void ScheduleQuestionTimeout(
        const boost::uuids::uuid& game_id,
        std::chrono::seconds timeout,
        std::function<void()> callback
    );
    
    // Отмена таймера
    void CancelTimer(const boost::uuids::uuid& game_id);
    
    // Проверка активных таймеров (для восстановления)
    void CheckExpiredTimers();
    
private:
    struct TimerInfo {
        boost::uuids::uuid game_id;
        userver::engine::TaskWithResult<void> task;
        std::chrono::system_clock::time_point expires_at;
    };
    
    userver::engine::TaskProcessor& task_processor_;
    ClusterPtr pg_cluster_;
    
    std::unordered_map<boost::uuids::uuid, TimerInfo> active_timers_;
    std::shared_mutex timers_mutex_;
};

// Реализация
void TimerManager::ScheduleQuestionTimeout(
    const boost::uuids::uuid& game_id,
    std::chrono::seconds timeout,
    std::function<void()> callback
) {
    auto expires_at = std::chrono::system_clock::now() + timeout;
    
    // Сохраняем в БД для восстановления после перезагрузки
    pg_cluster_->Execute(
        "INSERT INTO quiz.active_timers "
        "(game_session_id, timer_type, expires_at) "
        "VALUES ($1, 'question_timeout', $2) "
        "ON CONFLICT (game_session_id, timer_type) "
        "DO UPDATE SET expires_at = $2",
        game_id, expires_at
    );
    
    // Создаем асинхронную задачу
    auto task = userver::engine::AsyncNoSpan(task_processor_, [=]() {
        // Ждем timeout
        userver::engine::SleepFor(timeout);
        
        // Проверяем, что таймер не был отменен
        {
            std::shared_lock lock(timers_mutex_);
            if (active_timers_.find(game_id) == active_timers_.end()) {
                return;  // Таймер был отменен
            }
        }
        
        // Выполняем callback
        try {
            callback();
            
            // Удаляем из БД
            pg_cluster_->Execute(
                "DELETE FROM quiz.active_timers "
                "WHERE game_session_id = $1 AND timer_type = 'question_timeout'",
                game_id
            );
        } catch (const std::exception& e) {
            LOG_ERROR() << "Timer callback failed: " << e.what();
        }
        
        // Удаляем из памяти
        {
            std::unique_lock lock(timers_mutex_);
            active_timers_.erase(game_id);
        }
    });
    
    // Сохраняем в памяти
    {
        std::unique_lock lock(timers_mutex_);
        active_timers_[game_id] = TimerInfo{
            .game_id = game_id,
            .task = std::move(task),
            .expires_at = expires_at
        };
    }
    
    LOG_INFO() << "Scheduled question timeout for game: " << game_id 
               << ", expires at: " << expires_at;
}

void TimerManager::CancelTimer(const boost::uuids::uuid& game_id) {
    std::unique_lock lock(timers_mutex_);
    
    auto it = active_timers_.find(game_id);
    if (it != active_timers_.end()) {
        // Отменяем задачу (она проверит и выйдет)
        active_timers_.erase(it);
        
        // Удаляем из БД
        pg_cluster_->Execute(
            "DELETE FROM quiz.active_timers WHERE game_session_id = $1",
            game_id
        );
        
        LOG_INFO() << "Cancelled timer for game: " << game_id;
    }
}

} // namespace
```

### Использование в QuizGame

```cpp
// src/logic/game/types/quiz_game.cpp
class QuizGame : public BaseGame {
public:
    void PresentQuestion(size_t question_index) {
        current_question_index_ = question_index;
        
        // Сохраняем в БД
        pg_cluster_->Execute(
            "UPDATE quiz.game_sessions "
            "SET current_question_index = $1, "
            "    question_started_at = NOW() "
            "WHERE id = $2",
            question_index, game_id_
        );
        
        // Запускаем таймер (30 секунд по умолчанию)
        timer_manager_->ScheduleQuestionTimeout(
            game_id_,
            std::chrono::seconds(30),
            [this]() {
                // Callback при истечении времени
                OnQuestionTimeout();
            }
        );
        
        // Уведомляем observers
        NotifyObservers(QuestionPresentedEvent{
            game_id_, 
            questions_[question_index],
            question_index,
            questions_.size()
        });
    }
    
    void OnQuestionTimeout() {
        LOG_INFO() << "Question timeout for game: " << game_id_;
        
        // Автоматически отправляем пустые ответы для игроков, которые не ответили
        auto players = GetPlayers();
        for (const auto& player : players) {
            if (!HasPlayerAnswered(player.id, current_question_index_)) {
                SubmitEmptyAnswer(player.id);
            }
        }
        
        // Переходим к следующему вопросу или завершаем игру
        if (current_question_index_ < questions_.size() - 1) {
            PresentQuestion(current_question_index_ + 1);
        } else {
            Finish();
        }
    }
    
private:
    std::shared_ptr<TimerManager> timer_manager_;
};
```

### API для ручного перехода

```cpp
// src/handlers/game/advance_question.hpp
class AdvanceQuestionHandler : public HttpHandlerBase {
public:
    auto HandleRequestThrow(...) const -> std::string override {
        auto game_id = Utils::StringToUuid(request.GetPathArg("game_id"));
        
        // Отменяем текущий таймер
        timer_manager_->CancelTimer(game_id);
        
        // Принудительно переходим к следующему вопросу
        auto game = game_service_->GetGame(game_id);
        if (auto quiz_game = std::dynamic_pointer_cast<QuizGame>(game)) {
            quiz_game->AdvanceToNextQuestion();
        }
        
        return FormatResponse("Question advanced");
    }
};
```

**API**:
```http
POST /games/{game_id}/advance
# Принудительно переходит к следующему вопросу
```

---

## Проблема 3: Real-time уведомления фронтенду

### Архитектура: Webhook + Polling

Есть несколько подходов для уведомления Telegram Bot:

#### Подход 1: Webhook (Рекомендуется)

```
Backend (game_userver) → HTTP POST → Telegram Bot Server → Telegram API
```

**Преимущества**:
- Мгновенные уведомления
- Не требует постоянного соединения
- Простая реализация

**Реализация**:

```cpp
// src/logic/game/observers/webhook_observer.hpp
namespace game_userver::logic::game::observers {

class WebhookObserver : public IGameObserver {
public:
    explicit WebhookObserver(
        userver::clients::http::Client& http_client,
        ClusterPtr pg_cluster
    );
    
    void OnEvent(const GameEvent& event) override;
    
private:
    void SendWebhook(
        const std::string& webhook_url,
        const nlohmann::json& payload
    );
    
    auto GetSubscribers(const boost::uuids::uuid& game_id)
        -> std::vector<Subscription>;
    
    userver::clients::http::Client& http_client_;
    ClusterPtr pg_cluster_;
};

void WebhookObserver::OnEvent(const GameEvent& event) {
    std::visit([this](const auto& e) {
        HandleEvent(e);
    }, event);
}

void WebhookObserver::HandleEvent(const QuestionPresentedEvent& event) {
    // Получаем всех подписчиков этой игры
    auto subscribers = GetSubscribers(event.game_session_id);
    
    // Формируем payload
    nlohmann::json payload = {
        {"event_type", "question_presented"},
        {"game_id", boost::uuids::to_string(event.game_session_id)},
        {"question", {
            {"id", boost::uuids::to_string(event.question.id)},
            {"text", event.question.text},
            {"image_url", event.question.image_url}
        }},
        {"question_index", event.question_index},
        {"total_questions", event.total_questions}
    };
    
    // Отправляем webhook каждому подписчику
    for (const auto& subscriber : subscribers) {
        if (subscriber.webhook_url.empty()) {
            continue;
        }
        
        SendWebhook(subscriber.webhook_url, payload);
    }
}

void WebhookObserver::SendWebhook(
    const std::string& webhook_url,
    const nlohmann::json& payload
) {
    try {
        auto request = http_client_.CreateRequest()
            .post(webhook_url)
            .data(payload.dump())
            .timeout(std::chrono::seconds(5))
            .retry(2);
        
        auto response = request.perform();
        
        if (response->status_code() != 200) {
            LOG_WARNING() << "Webhook failed: " << webhook_url 
                         << ", status: " << response->status_code();
        }
    } catch (const std::exception& e) {
        LOG_ERROR() << "Webhook error: " << e.what();
    }
}

auto WebhookObserver::GetSubscribers(const boost::uuids::uuid& game_id)
    -> std::vector<Subscription> {
    
    auto result = pg_cluster_->Execute(
        "SELECT subscriber_type, subscriber_id, webhook_url "
        "FROM quiz.game_subscriptions "
        "WHERE game_session_id = $1",
        game_id
    );
    
    std::vector<Subscription> subscribers;
    for (const auto& row : result) {
        subscribers.push_back({
            .subscriber_type = row["subscriber_type"].As<std::string>(),
            .subscriber_id = row["subscriber_id"].As<std::string>(),
            .webhook_url = row["webhook_url"].As<std::string>()
        });
    }
    
    return subscribers;
}

} // namespace
```

**Регистрация webhook**:

```http
POST /games/{game_id}/subscribe
{
  "subscriber_type": "telegram_bot",
  "subscriber_id": "123456789",  // chat_id
  "webhook_url": "https://your-bot-server.com/webhook/game-events"
}
```

**Telegram Bot Server получает webhook**:

```python
# telegram_bot/webhook_handler.py
from flask import Flask, request
import telegram

app = Flask(__name__)
bot = telegram.Bot(token="YOUR_BOT_TOKEN")

@app.route('/webhook/game-events', methods=['POST'])
def handle_game_event():
    data = request.json
    event_type = data['event_type']
    
    if event_type == 'question_presented':
        game_id = data['game_id']
        question = data['question']
        
        # Отправляем сообщение в Telegram
        chat_id = get_chat_id_for_game(game_id)
        
        bot.send_message(
            chat_id=chat_id,
            text=f"Новый вопрос: {question['text']}",
            reply_markup=create_answer_keyboard(question)
        )
    
    elif event_type == 'question_timeout':
        # Отключаем кнопки
        disable_answer_buttons(data['game_id'])
    
    return {'status': 'ok'}
```

#### Подход 2: Server-Sent Events (SSE)

```
Telegram Bot ← SSE Stream ← Backend (game_userver)
```

**Преимущества**:
- Постоянное соединение
- Автоматическое переподключение
- Поддержка в userver

**Реализация**:

```cpp
// src/handlers/game/events_stream.hpp
class GameEventsStreamHandler : public userver::server::handlers::HttpHandlerBase {
public:
    auto HandleRequestThrow(...) const -> std::string override {
        auto game_id = Utils::StringToUuid(request.GetPathArg("game_id"));
        
        // Устанавливаем SSE headers
        request.GetHttpResponse().SetHeader("Content-Type", "text/event-stream");
        request.GetHttpResponse().SetHeader("Cache-Control", "no-cache");
        request.GetHttpResponse().SetHeader("Connection", "keep-alive");
        
        // Создаем observer для этого соединения
        auto observer = std::make_shared<SSEObserver>(
            [&request](const std::string& event_data) {
                // Отправляем SSE событие
                request.GetHttpResponse().SendChunk(
                    "data: " + event_data + "\n\n"
                );
            }
        );
        
        // Подписываемся на события игры
        auto game = game_service_->GetGame(game_id);
        game->AddObserver(observer);
        
        // Держим соединение открытым
        while (!request.IsConnectionClosed()) {
            userver::engine::SleepFor(std::chrono::seconds(1));
            
            // Отправляем heartbeat
            request.GetHttpResponse().SendChunk(": heartbeat\n\n");
        }
        
        // Отписываемся при закрытии соединения
        game->RemoveObserver(observer);
        
        return "";
    }
};
```

**Telegram Bot подключается к SSE**:

```python
# telegram_bot/sse_client.py
import sseclient
import requests

def listen_game_events(game_id):
    url = f"http://backend:8080/games/{game_id}/events"
    
    response = requests.get(url, stream=True)
    client = sseclient.SSEClient(response)
    
    for event in client.events():
        data = json.loads(event.data)
        handle_game_event(data)
```

#### Подход 3: Polling (Простой, но неэффективный)

```
Telegram Bot → GET /games/{id}/events?since=timestamp → Backend
```

**Telegram Bot периодически опрашивает**:

```python
# telegram_bot/polling.py
import time

def poll_game_events(game_id):
    last_timestamp = None
    
    while True:
        url = f"http://backend:8080/games/{game_id}/events"
        if last_timestamp:
            url += f"?since={last_timestamp}"
        
        response = requests.get(url)
        events = response.json()['events']
        
        for event in events:
            handle_game_event(event)
            last_timestamp = event['timestamp']
        
        time.sleep(1)  # Опрос каждую секунду
```

---

## Рекомендуемая архитектура

```
┌─────────────────────────────────────────────────────────┐
│                    game_userver                         │
│                                                         │
│  ┌──────────────────────────────────────────────────┐  │
│  │  GameService (Singleton)                         │  │
│  │  - Восстанавливается из БД при старте           │  │
│  │  - Все состояние в PostgreSQL                   │  │
│  └────────────┬─────────────────────────────────────┘  │
│               │                                         │
│  ┌────────────▼─────────────────────────────────────┐  │
│  │  TimerManager                                    │  │
│  │  - Таймеры для вопросов (30 сек)               │  │
│  │  - Сохраняет в БД для восстановления           │  │
│  │  - Автоматический переход к следующему вопросу │  │
│  └────────────┬─────────────────────────────────────┘  │
│               │                                         │
│  ┌────────────▼─────────────────────────────────────┐  │
│  │  WebhookObserver                                 │  │
│  │  - Отправляет HTTP POST на webhook URL          │  │
│  │  - Уведомляет о событиях игры                   │  │
│  └────────────┬─────────────────────────────────────┘  │
│               │                                         │
└───────────────┼─────────────────────────────────────────┘
                │ HTTP POST
                ▼
┌─────────────────────────────────────────────────────────┐
│              Telegram Bot Server                        │
│                                                         │
│  ┌──────────────────────────────────────────────────┐  │
│  │  Webhook Handler                                 │  │
│  │  - Получает события от game_userver             │  │
│  │  - Отправляет сообщения в Telegram              │  │
│  │  - Управляет кнопками (включает/отключает)      │  │
│  └────────────┬─────────────────────────────────────┘  │
│               │                                         │
└───────────────┼─────────────────────────────────────────┘
                │ Telegram API
                ▼
         ┌──────────────┐
         │   Telegram   │
         │    Users     │
         └──────────────┘
```

---

## Итоговое решение

### 1. Устойчивость к перезагрузкам ✅
- Все состояние в PostgreSQL
- Восстановление при старте из БД
- Таймеры сохраняются и восстанавливаются

### 2. Таймер вместо проверки ✅
- `TimerManager` с асинхронными задачами
- Таймер на каждый вопрос (30 сек)
- Автоматический переход или ручной через API

### 3. Real-time уведомления ✅
- **Webhook** (рекомендуется): Backend → HTTP POST → Bot Server
- Bot Server управляет Telegram кнопками
- Простая интеграция, надежная доставка

### Преимущества решения:
- ✅ Переживает перезагрузки
- ✅ Масштабируется (можно добавить несколько bot servers)
- ✅ Разделение ответственности (backend = логика, bot = UI)
- ✅ Легко тестировать
- ✅ Работает с любым фронтендом (не только Telegram)

Хотите, чтобы я показал конкретную реализацию какой-то части?