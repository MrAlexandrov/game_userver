# Архитектура бэкенда игровой системы

## Обзор

Бэкенд представляет собой высокопроизводительный сервер на C++ с использованием фреймворка [userver](https://github.com/userver-framework/userver), предназначенный для проведения многопользовательских викторин в реальном времени.

## Технологический стек

- **Язык**: C++23
- **Фреймворк**: userver (асинхронный HTTP/gRPC сервер)
- **База данных**: PostgreSQL 14+
- **Протоколы**: HTTP REST API, gRPC
- **Сборка**: CMake 3.12+

---

## Архитектурные слои

### 1. Presentation Layer (Handlers)

**Расположение**: `src/handlers/`

Обрабатывает входящие HTTP/gRPC запросы и формирует ответы.

```
src/handlers/
├── game/                    # Игровые endpoints
│   ├── create_game_session.cpp
│   ├── add_player.cpp
│   ├── start_game.cpp
│   ├── get_game_state.cpp
│   ├── submit_answer.cpp
│   └── get_game_results.cpp
├── content_handling/        # Управление контентом
│   ├── pack/               # Паки вопросов
│   ├── question/           # Вопросы
│   └── variant/            # Варианты ответов
└── grpc/                   # gRPC сервисы
```

**Принципы**:
- Тонкие handlers - только валидация входных данных и маршрутизация
- Вся бизнес-логика делегируется в Logic Layer
- Dependency Injection через userver ComponentContext

### 2. Business Logic Layer

**Расположение**: `src/logic/`

Содержит бизнес-логику приложения.

```
src/logic/
├── game/
│   ├── game.hpp/cpp              # GameService - основной сервис
│   ├── game_observer.hpp         # Observer pattern для событий
│   ├── game_events.hpp           # Типы игровых событий
│   └── observers/                # Конкретные observers
│       ├── logging_observer.hpp
│       ├── notification_observer.hpp
│       └── statistics_observer.hpp
└── validators/
    ├── answer_validator.hpp      # Интерфейс валидатора
    └── validator_factory.hpp     # Factory для создания валидаторов
```

**Ключевые компоненты**:

#### GameService
Центральный сервис для управления игровыми сессиями:
- Создание игровых сессий
- Добавление игроков
- Управление ходом игры
- Валидация ответов
- Уведомление observers о событиях

#### Observer Pattern
Позволяет подписываться на игровые события:
- `GameSessionCreatedEvent` - создана игровая сессия
- `PlayerAddedEvent` - добавлен игрок
- `GameStartedEvent` - игра началась
- `AnswerSubmittedEvent` - отправлен ответ
- `QuestionAdvancedEvent` - переход к следующему вопросу
- `GameFinishedEvent` - игра завершена

#### Validator Factory
Создает валидаторы для разных типов вопросов:
- `MultipleChoiceValidator` - для вопросов с выбором варианта
- `FreeTextValidator` - для вопросов со свободным ответом
- `CustomValidator` - для кастомных типов

### 3. Data Access Layer (Storage)

**Расположение**: `src/storage/`

Абстракция над базой данных (Repository pattern).

```
src/storage/
├── game_sessions.hpp/cpp         # CRUD для игровых сессий
├── players.hpp/cpp               # CRUD для игроков
├── player_answers.hpp/cpp        # CRUD для ответов игроков
├── packs.hpp/cpp                 # CRUD для паков
├── questions.hpp/cpp             # CRUD для вопросов
├── variants.hpp/cpp              # CRUD для вариантов
└── questions_and_variants.hpp/cpp # Комплексные запросы
```

**SQL запросы**: `src/queries/*.sql`

Все SQL-запросы вынесены в отдельные файлы для:
- Лучшей читаемости
- Возможности оптимизации без перекомпиляции
- Версионирования схемы БД

### 4. Domain Models

**Расположение**: `src/models/`

Доменные модели данных:
- `GameSession` - игровая сессия
- `Player` - игрок
- `PlayerAnswer` - ответ игрока
- `Pack` - пак вопросов
- `Question` - вопрос
- `Variant` - вариант ответа
- `QuestionType` - тип вопроса (enum)

---

## Компонентная архитектура (userver)

### Singleton Components

#### GameServiceComponent
**Файл**: `src/components/game_service/game_service_component.hpp`

Singleton компонент, обеспечивающий единственный экземпляр GameService на весь сервер.

**Преимущества**:
- Сохранение состояния между запросами
- Observers работают в реальном времени
- Нет overhead на создание/уничтожение объектов

**Конфигурация**: `configs/static_config.yaml`
```yaml
game-service:
    # Singleton GameService component
    # No additional configuration needed
```

**Регистрация**: `src/main.cpp`
```cpp
.Append<game_userver::components::GameServiceComponent>()
```

### Dependency Injection

Все handlers получают GameService через DI:

```cpp
struct Handler::Impl {
    logic::game::GameService& game_service;
    
    explicit Impl(const ComponentContext& context)
        : game_service(context
              .FindComponent<GameServiceComponent>()
              .GetGameService()) {}
};
```

---

## Поток данных

### Типичный запрос

```
┌─────────────┐
│   Client    │
└──────┬──────┘
       │ HTTP POST /games/{game_id}/answers
       ▼
┌─────────────────────────────────┐
│  Handler (submit_answer.cpp)    │
│  - Парсинг JSON                 │
│  - Валидация входных данных     │
└──────┬──────────────────────────┘
       │ impl_->game_service.SubmitAnswer(...)
       ▼
┌─────────────────────────────────┐
│  GameService (game.cpp)         │
│  - Получение игрока из БД       │
│  - Получение текущего вопроса   │
│  - Валидация ответа             │
│  - Сохранение ответа            │
│  - Обновление счета             │
│  - Уведомление observers        │
│  - Проверка завершения игры     │
└──────┬──────────────────────────┘
       │ NStorage::SubmitPlayerAnswer(...)
       ▼
┌─────────────────────────────────┐
│  Storage Layer                  │
│  - Выполнение SQL запросов      │
└──────┬──────────────────────────┘
       │ SQL: INSERT INTO player_answers
       ▼
┌─────────────────────────────────┐
│  PostgreSQL Database            │
└─────────────────────────────────┘
```

### Обработка событий (Observer Pattern)

```
┌─────────────────────────────────┐
│  GameService                    │
│  NotifyObservers(event)         │
└──────┬──────────────────────────┘
       │
       ├──────────────────────────┐
       │                          │
       ▼                          ▼
┌──────────────────┐    ┌──────────────────┐
│ LoggingObserver  │    │ NotificationObs  │
│ - Логирование    │    │ - Push уведомл.  │
└──────────────────┘    └──────────────────┘
       │                          │
       ▼                          ▼
┌──────────────────┐    ┌──────────────────┐
│  Logs/Metrics    │    │  WebSocket/SSE   │
└──────────────────┘    └──────────────────┘
```

---

## Конкурентность и безопасность

### Thread Safety

- **GameService**: Thread-safe благодаря userver task processors
- **PostgreSQL**: Connection pooling обеспечивает безопасный доступ
- **Observers**: Вызываются синхронно в контексте запроса

### Транзакции

⚠️ **TODO**: Добавить транзакции в критичные операции (см. REFACTORING_SUMMARY.md)

Текущая проблема:
```cpp
// В SubmitAnswer() несколько операций без транзакции:
auto player_answer = NStorage::SubmitPlayerAnswer(...);  // 1
NStorage::UpdatePlayerScore(...);                        // 2
NStorage::AdvanceToNextQuestion(...);                    // 3
// Если 2 или 3 упадет - несогласованное состояние!
```

Решение:
```cpp
auto trx = pg_cluster_->Begin(...);
try {
    // Все операции в транзакции
    trx.Commit();
} catch (...) {
    trx.Rollback();
    throw;
}
```

### Race Conditions

⚠️ **TODO**: Исправить race condition в проверке "все игроки ответили"

Текущая проблема:
```cpp
auto answers_count = GetAnswersCountForQuestion(...);
bool all_answered = (answers_count >= players.size());
// ⚠️ Между проверкой и действием может прийти еще ответ!
if (all_answered) {
    AdvanceToNextQuestion(...);
}
```

Решение: PostgreSQL advisory locks или SELECT FOR UPDATE

---

## Масштабируемость

### Вертикальное масштабирование
✅ Поддерживается:
- Увеличение worker threads в конфигурации
- Connection pooling для PostgreSQL
- Асинхронная обработка запросов

### Горизонтальное масштабирование
⚠️ Ограничения:
- GameService - singleton на каждом инстансе
- Нет distributed state (Redis/etcd)
- Observers не работают между инстансами

**Решение для будущего**:
- Вынести состояние игр в Redis
- Использовать Redis Pub/Sub для событий
- Sticky sessions или consistent hashing

---

## Мониторинг и отладка

### Логирование

Используется userver logging:
```cpp
LOG_INFO() << "Game session created: " << game_session_id;
LOG_WARNING() << "Player not found: " << player_id;
LOG_ERROR() << "Database error: " << e.what();
```

Уровни логирования настраиваются в `configs/static_config.yaml`:
```yaml
logging:
    loggers:
        default:
            level: $logger-level  # debug, info, warning, error
```

### Метрики

⚠️ **TODO**: Добавить Prometheus metrics:
- Количество активных игр
- Среднее время ответа
- Количество ошибок
- Latency запросов

### Трейсинг

userver поддерживает OpenTelemetry для distributed tracing.

---

## Безопасность

### Текущие меры:
- Валидация всех входных данных
- Параметризованные SQL запросы (защита от SQL injection)
- UUID для идентификаторов (защита от enumeration)

### TODO:
- Rate limiting на endpoints
- Authentication/Authorization
- CORS настройки
- Input sanitization для текстовых ответов

---

## Производительность

### Оптимизации:
- ✅ Асинхронная обработка запросов (userver)
- ✅ Connection pooling для PostgreSQL
- ✅ Singleton GameService (нет overhead на создание)
- ✅ Индексы в БД на внешние ключи

### TODO:
- Кэширование активных игр в памяти
- Batch операции для множественных ответов
- Read replicas для PostgreSQL
- CDN для статического контента (изображения вопросов)

---

## Тестирование

### Unit тесты
**Расположение**: `tests/unit/`

Тестируют отдельные компоненты:
- Валидаторы
- Утилиты (string_to_uuid, etc.)
- Модели данных

### Integration тесты
**Расположение**: `tests/handlers/`

Тестируют API endpoints:
- Создание паков
- Создание вопросов
- Игровой процесс

**Фреймворк**: pytest + userver testsuite

### Запуск тестов:
```bash
make test-debug
```

---

## Развертывание

### Docker
```bash
docker-compose up -d
```

Включает:
- Приложение (game_userver)
- PostgreSQL
- Миграции БД

### Конфигурация

Переменные окружения в `configs/config_vars.yaml`:
- `server-port`: HTTP порт (default: 8080)
- `grpc-server-port`: gRPC порт (default: 8081)
- `pg-connection`: PostgreSQL connection string
- `worker-threads`: Количество worker threads
- `logger-level`: Уровень логирования

---

## Дальнейшее развитие

### Краткосрочные цели:
1. Добавить транзакции в критичные операции
2. Улучшить обработку ошибок и логирование
3. Исправить race conditions
4. Добавить rate limiting

### Среднесрочные цели:
1. WebSocket для real-time уведомлений
2. Кэширование активных игр
3. Prometheus metrics
4. API versioning

### Долгосрочные цели:
1. Горизонтальное масштабирование (Redis)
2. Микросервисная архитектура
3. Event sourcing для истории игр
4. Machine learning для рекомендаций вопросов