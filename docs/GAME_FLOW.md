# Процесс игры - Пошаговая инструкция

Этот документ описывает полную последовательность API-вызовов для проведения игровой сессии.

## Общая схема процесса

```
1. Создание пака (YAML)
   ↓
2. Создание игровой сессии
   ↓
3. Добавление игроков
   ↓
4. Старт игры
   ↓
5. Цикл: Получение состояния → Отправка ответов
   ↓
6. Получение результатов
```

---

## Шаг 1: Создание пака из YAML

Сначала создаем пак с вопросами и вариантами ответов.

**Запрос:**
```bash
curl -X POST http://localhost:8080/packs/yaml \
  -H "Content-Type: text/plain" \
  -d '
title: Викторина по географии
questions:
  - text: Столица России
    variants:
      - text: Санкт-Петербург
      - text: Москва
        is_correct: true
      - text: Казань
      - text: Кострома
  
  - text: Самая длинная река в мире
    variants:
      - text: Нил
        is_correct: true
      - text: Амазонка
      - text: Волга
      - text: Миссисипи
  
  - text: Самая высокая гора в мире
    variants:
      - text: Килиманджаро
      - text: Эльбрус
      - text: Эверест
        is_correct: true
      - text: Монблан
'
```

**Ответ:**
```json
{
  "id": "06e5f172-107b-4b04-a7b5-8ee16e67f07d",
  "title": "Викторина по географии",
  "message": "Pack created successfully from YAML"
}
```

**Сохраните `pack_id`:** `06e5f172-107b-4b04-a7b5-8ee16e67f07d`

---

## Шаг 2: Создание игровой сессии

Создаем новую игровую сессию с выбранным паком.

**Запрос:**
```bash
curl -X POST http://localhost:8080/games \
  -H "Content-Type: application/json" \
  -d '{
    "pack_id": "06e5f172-107b-4b04-a7b5-8ee16e67f07d"
  }'
```

**Ответ:**
```json
{
  "id": "70b45b09-fe0d-4b98-9e35-62701c9e40c7",
  "pack_id": "06e5f172-107b-4b04-a7b5-8ee16e67f07d",
  "status": "waiting",
  "current_question_index": 0,
  "created_at": "2026-01-08T11:00:00Z"
}
```

**Сохраните `game_id`:** `70b45b09-fe0d-4b98-9e35-62701c9e40c7`

---

## Шаг 3: Добавление игроков

Добавляем игроков в игровую сессию. Можно добавить несколько игроков.

### Игрок 1

**Запрос:**
```bash
curl -X POST http://localhost:8080/games/70b45b09-fe0d-4b98-9e35-62701c9e40c7/players \
  -H "Content-Type: application/json" \
  -d '{
    "player_name": "Алексей"
  }'
```

**Ответ:**
```json
{
  "id": "54032ea5-e21e-4a9b-a02b-488c9922075d",
  "game_session_id": "70b45b09-fe0d-4b98-9e35-62701c9e40c7",
  "player_name": "Алексей",
  "score": 0
}
```

**Сохраните `player_id`:** `54032ea5-e21e-4a9b-a02b-488c9922075d`

### Игрок 2

**Запрос:**
```bash
curl -X POST http://localhost:8080/games/70b45b09-fe0d-4b98-9e35-62701c9e40c7/players \
  -H "Content-Type: application/json" \
  -d '{
    "player_name": "Мария"
  }'
```

**Ответ:**
```json
{
  "id": "1a08cb68-de55-46a5-9a50-d954d3bf12cb",
  "game_session_id": "70b45b09-fe0d-4b98-9e35-62701c9e40c7",
  "player_name": "Мария",
  "score": 0
}
```

**Сохраните `player_id`:** `1a08cb68-de55-46a5-9a50-d954d3bf12cb`

---

## Шаг 4: Старт игры

Запускаем игровую сессию. После этого игроки могут начать отвечать на вопросы.

**Запрос:**
```bash
curl -X POST http://localhost:8080/games/70b45b09-fe0d-4b98-9e35-62701c9e40c7/start
```

**Ответ:**
```json
{
  "id": "70b45b09-fe0d-4b98-9e35-62701c9e40c7",
  "status": "in_progress",
  "current_question_index": 0,
  "message": "Game started successfully"
}
```

---

## Шаг 5: Игровой процесс

### 5.1. Получение текущего состояния игры

Получаем информацию о текущем вопросе и вариантах ответа.

**Запрос:**
```bash
curl http://localhost:8080/games/70b45b09-fe0d-4b98-9e35-62701c9e40c7/state
```

**Ответ:**
```json
{
  "game_session": {
    "id": "70b45b09-fe0d-4b98-9e35-62701c9e40c7",
    "pack_id": "06e5f172-107b-4b04-a7b5-8ee16e67f07d",
    "status": "in_progress",
    "current_question_index": 0
  },
  "current_question": {
    "id": "880e8400-e29b-41d4-a716-446655440004",
    "pack_id": "06e5f172-107b-4b04-a7b5-8ee16e67f07d",
    "text": "Столица России",
    "image_url": ""
  },
  "variants": [
    {
      "id": "990e8400-e29b-41d4-a716-446655440005",
      "question_id": "880e8400-e29b-41d4-a716-446655440004",
      "text": "Санкт-Петербург"
    },
    {
      "id": "61be4e2f-7892-489c-909b-93b34263c481",
      "question_id": "880e8400-e29b-41d4-a716-446655440004",
      "text": "Москва"
    },
    {
      "id": "990e8400-e29b-41d4-a716-446655440007",
      "question_id": "880e8400-e29b-41d4-a716-446655440004",
      "text": "Казань"
    },
    {
      "id": "990e8400-e29b-41d4-a716-446655440008",
      "question_id": "880e8400-e29b-41d4-a716-446655440004",
      "text": "Кострома"
    }
  ],
  "players": [
    {
      "id": "54032ea5-e21e-4a9b-a02b-488c9922075d",
      "player_name": "Алексей",
      "score": 0
    },
    {
      "id": "1a08cb68-de55-46a5-9a50-d954d3bf12cb",
      "player_name": "Мария",
      "score": 0
    }
  ]
}
```

### 5.2. Отправка ответа игрока

Каждый игрок отправляет свой ответ на текущий вопрос.

#### Ответ игрока "Алексей"

**Запрос:**
```bash
curl -X POST http://localhost:8080/games/70b45b09-fe0d-4b98-9e35-62701c9e40c7/answers \
  -H "Content-Type: application/json" \
  -d '{
    "player_id": "54032ea5-e21e-4a9b-a02b-488c9922075d",
    "variant_id": "61be4e2f-7892-489c-909b-93b34263c481"
  }'
```

**Ответ:**
```json
{
  "is_correct": true,
  "message": "Correct answer!",
  "player_score": 1
}
```

#### Ответ игрока "Мария"

**Запрос:**
```bash
curl -X POST http://localhost:8080/games/70b45b09-fe0d-4b98-9e35-62701c9e40c7/answers \
  -H "Content-Type: application/json" \
  -d '{
    "player_id": "1a08cb68-de55-46a5-9a50-d954d3bf12cb",
    "variant_id": "990e8400-e29b-41d4-a716-446655440005"
  }'
```

**Ответ:**
```json
{
  "is_correct": false,
  "message": "Wrong answer!",
  "player_score": 0
}
```

### 5.3. Повторение цикла

После того как все игроки ответили, повторяем шаги 5.1 и 5.2 для следующих вопросов:

1. **Получаем состояние** → видим следующий вопрос (current_question_index увеличивается)
2. **Отправляем ответы** всех игроков
3. Повторяем до конца всех вопросов

---

## Шаг 6: Получение результатов игры

После завершения всех вопросов получаем финальные результаты.

**Запрос:**
```bash
curl http://localhost:8080/games/70b45b09-fe0d-4b98-9e35-62701c9e40c7/results
```

**Ответ:**
```json
{
  "game_session": {
    "id": "70b45b09-fe0d-4b98-9e35-62701c9e40c7",
    "pack_id": "06e5f172-107b-4b04-a7b5-8ee16e67f07d",
    "status": "finished",
    "current_question_index": 3
  },
  "players": [
    {
      "id": "54032ea5-e21e-4a9b-a02b-488c9922075d",
      "player_name": "Алексей",
      "score": 3,
      "rank": 1
    },
    {
      "id": "1a08cb68-de55-46a5-9a50-d954d3bf12cb",
      "player_name": "Мария",
      "score": 2,
      "rank": 2
    }
  ],
  "total_questions": 3
}
```

---

## Полный пример скрипта (bash)

```bash
#!/bin/bash

# Цвета для вывода
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

BASE_URL="http://localhost:8080"

echo -e "${BLUE}=== Шаг 1: Создание пака ===${NC}"
PACK_RESPONSE=$(curl -s -X POST $BASE_URL/packs/yaml \
  -H "Content-Type: text/plain" \
  -d '
title: Викторина по географии
questions:
  - text: Столица России
    variants:
      - text: Санкт-Петербург
      - text: Москва
        is_correct: true
      - text: Казань
  - text: Самая длинная река в мире
    variants:
      - text: Нил
        is_correct: true
      - text: Амазонка
      - text: Волга
')

PACK_ID=$(echo $PACK_RESPONSE | jq -r '.id')
echo -e "${GREEN}Pack ID: $PACK_ID${NC}"

echo -e "\n${BLUE}=== Шаг 2: Создание игровой сессии ===${NC}"
GAME_RESPONSE=$(curl -s -X POST $BASE_URL/games \
  -H "Content-Type: application/json" \
  -d "{\"pack_id\": \"$PACK_ID\"}")

GAME_ID=$(echo $GAME_RESPONSE | jq -r '.id')
echo -e "${GREEN}Game ID: $GAME_ID${NC}"

echo -e "\n${BLUE}=== Шаг 3: Добавление игроков ===${NC}"
PLAYER1_RESPONSE=$(curl -s -X POST $BASE_URL/games/$GAME_ID/players \
  -H "Content-Type: application/json" \
  -d '{"player_name": "Алексей"}')

PLAYER1_ID=$(echo $PLAYER1_RESPONSE | jq -r '.id')
echo -e "${GREEN}Player 1 ID: $PLAYER1_ID (Алексей)${NC}"

PLAYER2_RESPONSE=$(curl -s -X POST $BASE_URL/games/$GAME_ID/players \
  -H "Content-Type: application/json" \
  -d '{"player_name": "Мария"}')

PLAYER2_ID=$(echo $PLAYER2_RESPONSE | jq -r '.id')
echo -e "${GREEN}Player 2 ID: $PLAYER2_ID (Мария)${NC}"

echo -e "\n${BLUE}=== Шаг 4: Старт игры ===${NC}"
curl -s -X POST $BASE_URL/games/$GAME_ID/start | jq '.'

echo -e "\n${BLUE}=== Шаг 5: Игровой процесс ===${NC}"

# Получаем количество вопросов
STATE=$(curl -s $BASE_URL/games/$GAME_ID/state)
TOTAL_QUESTIONS=$(echo $STATE | jq '.variants | length')

for i in $(seq 0 1); do
  echo -e "\n${BLUE}--- Вопрос $((i+1)) ---${NC}"
  
  # Получаем состояние
  STATE=$(curl -s $BASE_URL/games/$GAME_ID/state)
  echo $STATE | jq '.current_question'
  
  # Получаем ID первого варианта (для примера)
  VARIANT_ID=$(echo $STATE | jq -r '.variants[1].id')
  
  # Игрок 1 отвечает
  echo -e "\n${GREEN}Алексей отвечает...${NC}"
  curl -s -X POST $BASE_URL/games/$GAME_ID/answers \
    -H "Content-Type: application/json" \
    -d "{\"player_id\": \"$PLAYER1_ID\", \"variant_id\": \"$VARIANT_ID\"}" | jq '.'
  
  # Игрок 2 отвечает
  echo -e "\n${GREEN}Мария отвечает...${NC}"
  curl -s -X POST $BASE_URL/games/$GAME_ID/answers \
    -H "Content-Type: application/json" \
    -d "{\"player_id\": \"$PLAYER2_ID\", \"variant_id\": \"$VARIANT_ID\"}" | jq '.'
  
  sleep 1
done

echo -e "\n${BLUE}=== Шаг 6: Результаты игры ===${NC}"
curl -s $BASE_URL/games/$GAME_ID/results | jq '.'
```

Сохраните этот скрипт как `game_flow.sh`, дайте права на выполнение (`chmod +x game_flow.sh`) и запустите (`./game_flow.sh`).

---

## Важные замечания

1. **Порядок вызовов**: Строго следуйте последовательности шагов
2. **ID сущностей**: Сохраняйте все ID (pack_id, game_id, player_id, variant_id) для последующих запросов
3. **Статус игры**: Проверяйте статус игры перед отправкой ответов
4. **Все игроки должны ответить**: Перед переходом к следующему вопросу убедитесь, что все игроки ответили
5. **Завершение игры**: Игра автоматически завершается после последнего вопроса

---

## Дополнительные эндпоинты

### Получить информацию о паке
```bash
curl http://localhost:8080/packs/{pack_id}
```

### Получить все вопросы пака
```bash
curl http://localhost:8080/packs/{pack_id}/questions
```

### Получить варианты ответа для вопроса
```bash
curl http://localhost:8080/questions/{question_id}/variants