# Быстрое руководство по рефакторингу

## ✅ Что уже сделано

1. Обновлена конфигурация [`configs/static_config.yaml`](configs/static_config.yaml) - все endpoints теперь используют RESTful пути с path parameters

## 🔧 Что нужно сделать

### Ключевое изменение в коде

Во всех handlers нужно заменить:

**Для GET запросов с query parameters:**
```cpp
// Было:
auto id_str = request.GetArg("id");

// Стало:
auto id_str = request.GetPathArg("pack_id");  // или game_id, question_id и т.д.
```

**Для POST запросов где ID был в JSON:**
```cpp
// Было:
auto json = userver::formats::json::FromString(request.RequestBody());
auto pack_id_str = json["pack_id"].As<std::string>();

// Стало:
auto pack_id_str = request.GetPathArg("pack_id");
auto json = userver::formats::json::FromString(request.RequestBody());
// pack_id больше не нужен в JSON
```

### Список файлов для изменения

#### Content Management Handlers

1. **src/handlers/content_handling/pack/get_pack_by_id.cpp**
   - Заменить `request.GetArg("id")` на `request.GetPathArg("pack_id")`

2. **src/handlers/content_handling/question/create_question.cpp**
   - Добавить `auto pack_id_str = request.GetPathArg("pack_id");`
   - Убрать `pack_id` из парсинга JSON

3. **src/handlers/content_handling/question/get_question_by_id.cpp**
   - Заменить `request.GetArg("id")` на `request.GetPathArg("question_id")`

4. **src/handlers/content_handling/question/get_questions_by_pack_id.cpp**
   - Заменить `request.GetArg("pack_id")` на `request.GetPathArg("pack_id")`

5. **src/handlers/content_handling/variant/create_variant.cpp**
   - Добавить `auto question_id_str = request.GetPathArg("question_id");`
   - Убрать `question_id` из парсинга JSON

6. **src/handlers/content_handling/variant/get_variant_by_id.cpp**
   - Заменить `request.GetArg("id")` на `request.GetPathArg("variant_id")`

7. **src/handlers/content_handling/variant/get_variants_by_question_id.cpp**
   - Заменить `request.GetArg("question_id")` на `request.GetPathArg("question_id")`

#### Game Handlers

8. **src/handlers/game/add_player.cpp**
   - Добавить `auto game_session_id_str = request.GetPathArg("game_id");`
   - Убрать `game_session_id` из парсинга JSON

9. **src/handlers/game/start_game.cpp**
   - Заменить парсинг JSON на `auto game_session_id_str = request.GetPathArg("game_id");`

10. **src/handlers/game/get_current_question.cpp** → **get_game_state.cpp**
    - Переименовать файлы (.hpp и .cpp)
    - Переименовать класс `GetCurrentQuestion` → `GetGameState`
    - Заменить парсинг JSON на `auto game_session_id_str = request.GetPathArg("game_id");`
    - Обновить `kName` на `"handler-get-game-state"`

11. **src/handlers/game/submit_answer.cpp**
    - Добавить `auto game_session_id_str = request.GetPathArg("game_id");`
    - Оставить `player_id` и `variant_id` в JSON

12. **src/handlers/game/get_game_results.cpp**
    - Заменить парсинг JSON на `auto game_session_id_str = request.GetPathArg("game_id");`

### Обновить CMakeLists.txt

```cmake
# Строка ~127:
# Было:
src/handlers/game/get_current_question.cpp

# Стало:
src/handlers/game/get_game_state.cpp
```

### Обновить component_list

**src/handlers/game/component_list.cpp:**
```cpp
// Было:
#include "get_current_question.hpp"
component_list.Append<GetCurrentQuestion>();

// Стало:
#include "get_game_state.hpp"
component_list.Append<GetGameState>();
```

### Обновить Telegram бота

**telegram_bot.py** - обновить все URL:

```python
# Было:
f'{API_BASE_URL}/game/session'
f'{API_BASE_URL}/game/player'
f'{API_BASE_URL}/game/start'
f'{API_BASE_URL}/game/question'
f'{API_BASE_URL}/game/answer'
f'{API_BASE_URL}/game/results'

# Стало:
f'{API_BASE_URL}/games'
f'{API_BASE_URL}/games/{game_id}/players'
f'{API_BASE_URL}/games/{game_id}/start'
f'{API_BASE_URL}/games/{game_id}/state'
f'{API_BASE_URL}/games/{game_id}/answers'
f'{API_BASE_URL}/games/{game_id}/results'
```

### Обновить create_test_data.py

```python
# Было:
f'{API_URL}/create-pack'
f'{API_URL}/create-question'
f'{API_URL}/create-variant'

# Стало:
f'{API_URL}/packs'
f'{API_URL}/packs/{pack_id}/questions'
f'{API_URL}/questions/{question_id}/variants'
```

## 📝 Примеры изменений

### Пример 1: get_pack_by_id.cpp

```cpp
// БЫЛО:
auto GetPackById::HandleRequestThrow(...) const -> std::string {
    auto pack_id_str = request.GetArg("id");
    auto pack_id = Utils::StringToUuid(pack_id_str);
    // ...
}

// СТАЛО:
auto GetPackById::HandleRequestThrow(...) const -> std::string {
    auto pack_id_str = request.GetPathArg("pack_id");
    auto pack_id = Utils::StringToUuid(pack_id_str);
    // ...
}
```

### Пример 2: create_question.cpp

```cpp
// БЫЛО:
auto CreateQuestion::HandleRequestThrow(...) const -> std::string {
    const auto& request_body = request.RequestBody();
    auto json = userver::formats::json::FromString(request_body);
    
    auto pack_id_str = json["pack_id"].As<std::string>();
    auto text = json["text"].As<std::string>();
    // ...
}

// СТАЛО:
auto CreateQuestion::HandleRequestThrow(...) const -> std::string {
    auto pack_id_str = request.GetPathArg("pack_id");
    
    const auto& request_body = request.RequestBody();
    auto json = userver::formats::json::FromString(request_body);
    
    auto text = json["text"].As<std::string>();
    // pack_id теперь из пути, не из JSON
    // ...
}
```

### Пример 3: start_game.cpp

```cpp
// БЫЛО:
auto StartGame::HandleRequestThrow(...) const -> std::string {
    const auto& request_body = request.RequestBody();
    auto json = userver::formats::json::FromString(request_body);
    
    auto game_session_id_str = json["game_session_id"].As<std::string>();
    // ...
}

// СТАЛО:
auto StartGame::HandleRequestThrow(...) const -> std::string {
    auto game_session_id_str = request.GetPathArg("game_id");
    // JSON body больше не нужен для этого endpoint
    // ...
}
```

## 🚀 Порядок действий

1. Сделайте backup проекта
2. Обновите handlers по одному, тестируя каждый
3. Обновите CMakeLists.txt
4. Обновите component_list
5. Пересоберите проект
6. Обновите Telegram бота
7. Обновите create_test_data.py
8. Протестируйте все endpoints

## ✅ Проверка

После изменений проверьте, что:
- Проект собирается без ошибок
- Все handlers зарегистрированы
- Telegram бот работает с новыми endpoints
- create_test_data.py создает данные корректно

## 📚 Дополнительные ресурсы

- Полное руководство: [`REFACTORING_GUIDE.md`](REFACTORING_GUIDE.md)
- Новая конфигурация: [`configs/static_config.yaml`](configs/static_config.yaml)
- Документация userver по path parameters: https://userver.tech/

## 💡 Совет

Используйте поиск и замену в вашей IDE для быстрого рефакторинга:
- `request.GetArg("id")` → `request.GetPathArg("pack_id")` (и т.д.)
- `json["pack_id"]` → проверьте, нужно ли это поле в JSON