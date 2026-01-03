# Руководство по рефакторингу API

## Обзор изменений

Проект переходит на полностью RESTful API с использованием path parameters вместо query parameters или JSON body для идентификаторов ресурсов.

## Изменения в handlers

### Content Management API

#### 1. Pack Handlers

**handler-get-pack-by-id**
- Старый путь: `GET /get-pack?id=uuid`
- Новый путь: `GET /packs/{pack_id}`
- Изменения в коде:
```cpp
// Было:
auto pack_id_str = request.GetArg("id");

// Стало:
auto pack_id_str = request.GetPathArg("pack_id");
```

**handler-create-pack**
- Старый путь: `POST /create-pack`
- Новый путь: `POST /packs`
- Изменения: только путь в config

**handler-get-all-packs**
- Старый путь: `GET /get-all-packs`
- Новый путь: `GET /packs`
- Изменения: только путь в config

#### 2. Question Handlers

**handler-create-question**
- Старый путь: `POST /create-question` (pack_id в JSON)
- Новый путь: `POST /packs/{pack_id}/questions`
- Изменения в коде:
```cpp
// Было:
auto json = userver::formats::json::FromString(request.RequestBody());
auto pack_id_str = json["pack_id"].As<std::string>();

// Стало:
auto pack_id_str = request.GetPathArg("pack_id");
auto json = userver::formats::json::FromString(request.RequestBody());
// pack_id больше не нужен в JSON
```

**handler-get-question-by-id**
- Старый путь: `GET /get-question-by-id?id=uuid`
- Новый путь: `GET /questions/{question_id}`
- Изменения в коде:
```cpp
// Было:
auto question_id_str = request.GetArg("id");

// Стало:
auto question_id_str = request.GetPathArg("question_id");
```

**handler-get-questions-by-pack-id**
- Старый путь: `GET /get-questions-by-pack-id?pack_id=uuid`
- Новый путь: `GET /packs/{pack_id}/questions`
- Изменения в коде:
```cpp
// Было:
auto pack_id_str = request.GetArg("pack_id");

// Стало:
auto pack_id_str = request.GetPathArg("pack_id");
```

#### 3. Variant Handlers

**handler-create-variant**
- Старый путь: `POST /create-variant` (question_id в JSON)
- Новый путь: `POST /questions/{question_id}/variants`
- Изменения в коде:
```cpp
// Было:
auto json = userver::formats::json::FromString(request.RequestBody());
auto question_id_str = json["question_id"].As<std::string>();

// Стало:
auto question_id_str = request.GetPathArg("question_id");
auto json = userver::formats::json::FromString(request.RequestBody());
// question_id больше не нужен в JSON
```

**handler-get-variant-by-id**
- Старый путь: `GET /get-variant-by-id?id=uuid`
- Новый путь: `GET /variants/{variant_id}`
- Изменения в коде:
```cpp
// Было:
auto variant_id_str = request.GetArg("id");

// Стало:
auto variant_id_str = request.GetPathArg("variant_id");
```

**handler-get-variants-by-question-id**
- Старый путь: `GET /get-variants-by-question-id?question_id=uuid`
- Новый путь: `GET /questions/{question_id}/variants`
- Изменения в коде:
```cpp
// Было:
auto question_id_str = request.GetArg("question_id");

// Стало:
auto question_id_str = request.GetPathArg("question_id");
```

### Game API

#### 1. Game Session Handlers

**handler-create-game-session**
- Старый путь: `POST /game/session`
- Новый путь: `POST /games`
- Изменения: только путь в config

**handler-add-player**
- Старый путь: `POST /game/player` (game_session_id в JSON)
- Новый путь: `POST /games/{game_id}/players`
- Изменения в коде:
```cpp
// Было:
auto json = userver::formats::json::FromString(request.RequestBody());
auto game_session_id_str = json["game_session_id"].As<std::string>();

// Стало:
auto game_session_id_str = request.GetPathArg("game_id");
auto json = userver::formats::json::FromString(request.RequestBody());
// game_session_id больше не нужен в JSON, только player_name
```

**handler-start-game**
- Старый путь: `POST /game/start` (game_session_id в JSON)
- Новый путь: `POST /games/{game_id}/start`
- Изменения в коде:
```cpp
// Было:
auto json = userver::formats::json::FromString(request.RequestBody());
auto game_session_id_str = json["game_session_id"].As<std::string>();

// Стало:
auto game_session_id_str = request.GetPathArg("game_id");
// JSON body больше не нужен
```

**handler-get-game-state** (НОВЫЙ - переименован из get-current-question)
- Старый путь: `POST /game/question` (game_session_id в JSON)
- Новый путь: `GET /games/{game_id}/state`
- Изменения в коде:
```cpp
// Было:
auto json = userver::formats::json::FromString(request.RequestBody());
auto game_session_id_str = json["game_session_id"].As<std::string>();

// Стало:
auto game_session_id_str = request.GetPathArg("game_id");
```
- Переименовать файлы:
  - `get_current_question.hpp` → `get_game_state.hpp`
  - `get_current_question.cpp` → `get_game_state.cpp`
  - Класс `GetCurrentQuestion` → `GetGameState`

**handler-submit-answer**
- Старый путь: `POST /game/answer` (player_id и variant_id в JSON)
- Новый путь: `POST /games/{game_id}/answers`
- Изменения в коде:
```cpp
// Было:
auto json = userver::formats::json::FromString(request.RequestBody());
auto player_id_str = json["player_id"].As<std::string>();
auto variant_id_str = json["variant_id"].As<std::string>();

// Стало:
auto game_session_id_str = request.GetPathArg("game_id");
auto json = userver::formats::json::FromString(request.RequestBody());
auto player_id_str = json["player_id"].As<std::string>();
auto variant_id_str = json["variant_id"].As<std::string>();
// game_id из пути, остальное из JSON
```

**handler-get-game-results**
- Старый путь: `POST /game/results` (game_session_id в JSON)
- Новый путь: `GET /games/{game_id}/results`
- Изменения в коде:
```cpp
// Было:
auto json = userver::formats::json::FromString(request.RequestBody());
auto game_session_id_str = json["game_session_id"].As<std::string>();

// Стало:
auto game_session_id_str = request.GetPathArg("game_id");
```

## Примеры использования новых endpoints

### Content Management

```bash
# Создать пак
curl -X POST http://localhost:8080/packs \
  -H "Content-Type: application/json" \
  -d '{"title": "Общие знания"}'

# Получить все паки
curl http://localhost:8080/packs

# Получить конкретный пак
curl http://localhost:8080/packs/550e8400-e29b-41d4-a716-446655440000

# Создать вопрос в паке
curl -X POST http://localhost:8080/packs/550e8400-e29b-41d4-a716-446655440000/questions \
  -H "Content-Type: application/json" \
  -d '{"text": "Какая столица России?", "image_url": null}'

# Получить все вопросы пака
curl http://localhost:8080/packs/550e8400-e29b-41d4-a716-446655440000/questions

# Получить конкретный вопрос
curl http://localhost:8080/questions/question-uuid

# Создать вариант ответа
curl -X POST http://localhost:8080/questions/question-uuid/variants \
  -H "Content-Type: application/json" \
  -d '{"text": "Москва", "is_correct": true}'

# Получить все варианты вопроса
curl http://localhost:8080/questions/question-uuid/variants

# Получить конкретный вариант
curl http://localhost:8080/variants/variant-uuid
```

### Game API

```bash
# Создать игру
curl -X POST http://localhost:8080/games \
  -H "Content-Type: application/json" \
  -d '{"pack_id": "pack-uuid"}'

# Добавить игрока
curl -X POST http://localhost:8080/games/game-uuid/players \
  -H "Content-Type: application/json" \
  -d '{"player_name": "Игрок 1"}'

# Начать игру
curl -X POST http://localhost:8080/games/game-uuid/start

# Получить состояние игры (текущий вопрос)
curl http://localhost:8080/games/game-uuid/state

# Отправить ответ
curl -X POST http://localhost:8080/games/game-uuid/answers \
  -H "Content-Type: application/json" \
  -d '{"player_id": "player-uuid", "variant_id": "variant-uuid"}'

# Получить результаты
curl http://localhost:8080/games/game-uuid/results
```

## Обновление CMakeLists.txt

Переименовать файлы в списке исходников:
```cmake
# Было:
src/handlers/game/get_current_question.cpp

# Стало:
src/handlers/game/get_game_state.cpp
```

## Обновление component_list

В `src/handlers/game/component_list.cpp`:
```cpp
// Было:
#include "get_current_question.hpp"
component_list.Append<GetCurrentQuestion>();

// Стало:
#include "get_game_state.hpp"
component_list.Append<GetGameState>();
```

## Обновление Telegram бота

Все вызовы API нужно обновить для использования новых путей с path parameters.

## Обновление тестов

Все тесты нужно обновить для использования новых endpoints.

## Обновление документации

- GAME_SETUP_GUIDE.md
- TELEGRAM_BOT_README.md
- API_CHANGES.md
- create_test_data.py

## Приоритет изменений

1. ✅ Обновлена конфигурация (static_config.yaml)
2. ⏳ Переименовать get_current_question → get_game_state
3. ⏳ Обновить все handlers для использования GetPathArg()
4. ⏳ Обновить CMakeLists.txt
5. ⏳ Обновить component_list
6. ⏳ Обновить Telegram бота
7. ⏳ Обновить create_test_data.py
8. ⏳ Обновить документацию