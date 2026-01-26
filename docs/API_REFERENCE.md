# API Reference

Полная документация REST API для игровой системы.

**Base URL**: `http://localhost:8080`

---

## Содержание

1. [Content Management API](#content-management-api)
   - [Packs](#packs)
   - [Questions](#questions)
   - [Variants](#variants)
2. [Game API](#game-api)
   - [Game Sessions](#game-sessions)
   - [Players](#players)
   - [Game Flow](#game-flow)
3. [Error Responses](#error-responses)
4. [Data Models](#data-models)

---

## Content Management API

### Packs

#### Create Pack from YAML

Создает пак вопросов из YAML-файла.

**Endpoint**: `POST /packs/yaml`

**Content-Type**: `text/plain`

**Request Body**:
```yaml
title: Викторина по географии
questions:
  - text: Столица России
    image_url: https://example.com/moscow.jpg  # опционально
    question_type: multiple_choice              # опционально, default: multiple_choice
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
```

**Response**: `200 OK`
```json
{
  "id": "06e5f172-107b-4b04-a7b5-8ee16e67f07d",
  "title": "Викторина по географии",
  "message": "Pack created successfully from YAML"
}
```

**Errors**:
- `400 Bad Request` - Invalid YAML format
- `500 Internal Server Error` - Database error

---

#### Create Pack (JSON)

Создает пустой пак вопросов.

**Endpoint**: `POST /packs`

**Request Body**:
```json
{
  "title": "Викторина по истории"
}
```

**Response**: `200 OK`
```json
{
  "id": "550e8400-e29b-41d4-a716-446655440000",
  "title": "Викторина по истории"
}
```

---

#### Get All Packs

Получает список всех паков.

**Endpoint**: `GET /packs`

**Response**: `200 OK`
```json
{
  "packs": [
    {
      "id": "06e5f172-107b-4b04-a7b5-8ee16e67f07d",
      "title": "Викторина по географии"
    },
    {
      "id": "550e8400-e29b-41d4-a716-446655440000",
      "title": "Викторина по истории"
    }
  ]
}
```

---

#### Get Pack by ID

Получает информацию о конкретном паке.

**Endpoint**: `GET /packs/{pack_id}`

**Path Parameters**:
- `pack_id` (UUID) - ID пака

**Response**: `200 OK`
```json
{
  "id": "06e5f172-107b-4b04-a7b5-8ee16e67f07d",
  "title": "Викторина по географии"
}
```

**Errors**:
- `404 Not Found` - Pack not found

---

### Questions

#### Create Question

Создает вопрос в паке.

**Endpoint**: `POST /packs/{pack_id}/questions`

**Path Parameters**:
- `pack_id` (UUID) - ID пака

**Request Body**:
```json
{
  "text": "Столица Франции?",
  "image_url": "https://example.com/paris.jpg",
  "question_type": "multiple_choice"
}
```

**Fields**:
- `text` (string, required) - Текст вопроса
- `image_url` (string, optional) - URL изображения
- `question_type` (string, optional) - Тип вопроса: `multiple_choice`, `free_text`, `custom`

**Response**: `200 OK`
```json
{
  "id": "880e8400-e29b-41d4-a716-446655440004",
  "pack_id": "06e5f172-107b-4b04-a7b5-8ee16e67f07d",
  "text": "Столица Франции?",
  "image_url": "https://example.com/paris.jpg",
  "question_type": "multiple_choice"
}
```

---

#### Get Question by ID

**Endpoint**: `GET /questions/{question_id}`

**Response**: `200 OK`
```json
{
  "id": "880e8400-e29b-41d4-a716-446655440004",
  "pack_id": "06e5f172-107b-4b04-a7b5-8ee16e67f07d",
  "text": "Столица Франции?",
  "image_url": "https://example.com/paris.jpg",
  "question_type": "multiple_choice"
}
```

---

#### Get Questions by Pack ID

Получает все вопросы пака.

**Endpoint**: `GET /packs/{pack_id}/questions`

**Response**: `200 OK`
```json
{
  "questions": [
    {
      "id": "880e8400-e29b-41d4-a716-446655440004",
      "pack_id": "06e5f172-107b-4b04-a7b5-8ee16e67f07d",
      "text": "Столица России",
      "image_url": "",
      "question_type": "multiple_choice"
    }
  ]
}
```

---

### Variants

#### Create Variant

Создает вариант ответа для вопроса.

**Endpoint**: `POST /questions/{question_id}/variants`

**Request Body**:
```json
{
  "text": "Париж",
  "is_correct": true
}
```

**Response**: `200 OK`
```json
{
  "id": "990e8400-e29b-41d4-a716-446655440005",
  "question_id": "880e8400-e29b-41d4-a716-446655440004",
  "text": "Париж",
  "is_correct": true
}
```

---

#### Get Variant by ID

**Endpoint**: `GET /variants/{variant_id}`

**Response**: `200 OK`
```json
{
  "id": "990e8400-e29b-41d4-a716-446655440005",
  "question_id": "880e8400-e29b-41d4-a716-446655440004",
  "text": "Париж",
  "is_correct": true
}
```

---

#### Get Variants by Question ID

**Endpoint**: `GET /questions/{question_id}/variants`

**Response**: `200 OK`
```json
{
  "variants": [
    {
      "id": "990e8400-e29b-41d4-a716-446655440005",
      "question_id": "880e8400-e29b-41d4-a716-446655440004",
      "text": "Париж",
      "is_correct": true
    },
    {
      "id": "990e8400-e29b-41d4-a716-446655440006",
      "question_id": "880e8400-e29b-41d4-a716-446655440004",
      "text": "Лондон",
      "is_correct": false
    }
  ]
}
```

---

## Game API

### Game Sessions

#### Create Game Session

Создает новую игровую сессию.

**Endpoint**: `POST /games`

**Request Body**:
```json
{
  "pack_id": "06e5f172-107b-4b04-a7b5-8ee16e67f07d"
}
```

**Response**: `200 OK`
```json
{
  "id": "70b45b09-fe0d-4b98-9e35-62701c9e40c7",
  "pack_id": "06e5f172-107b-4b04-a7b5-8ee16e67f07d",
  "state": "waiting",
  "current_question_index": 0
}
```

**States**:
- `waiting` - Ожидание игроков
- `active` - Игра идет
- `finished` - Игра завершена

---

#### Get Game State

Получает текущее состояние игры.

**Endpoint**: `GET /games/{game_id}/state`

**Response**: `200 OK`
```json
{
  "game_session": {
    "id": "70b45b09-fe0d-4b98-9e35-62701c9e40c7",
    "pack_id": "06e5f172-107b-4b04-a7b5-8ee16e67f07d",
    "state": "active",
    "current_question_index": 0
  },
  "current_question": {
    "id": "880e8400-e29b-41d4-a716-446655440004",
    "pack_id": "06e5f172-107b-4b04-a7b5-8ee16e67f07d",
    "text": "Столица России",
    "image_url": "",
    "question_type": "multiple_choice"
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
    }
  ],
  "players": [
    {
      "id": "54032ea5-e21e-4a9b-a02b-488c9922075d",
      "game_session_id": "70b45b09-fe0d-4b98-9e35-62701c9e40c7",
      "name": "Алексей",
      "score": 0
    }
  ]
}
```

**Note**: Варианты ответов возвращаются БЕЗ поля `is_correct` для предотвращения читерства.

---

#### Get Game Results

Получает результаты завершенной игры.

**Endpoint**: `GET /games/{game_id}/results`

**Response**: `200 OK`
```json
{
  "game_session": {
    "id": "70b45b09-fe0d-4b98-9e35-62701c9e40c7",
    "pack_id": "06e5f172-107b-4b04-a7b5-8ee16e67f07d",
    "state": "finished",
    "current_question_index": 3
  },
  "players": [
    {
      "id": "54032ea5-e21e-4a9b-a02b-488c9922075d",
      "name": "Алексей",
      "score": 3,
      "rank": 1
    },
    {
      "id": "1a08cb68-de55-46a5-9a50-d954d3bf12cb",
      "name": "Мария",
      "score": 2,
      "rank": 2
    }
  ],
  "total_questions": 3
}
```

---

### Players

#### Add Player

Добавляет игрока в игровую сессию.

**Endpoint**: `POST /games/{game_id}/players`

**Request Body**:
```json
{
  "player_name": "Алексей"
}
```

**Response**: `200 OK`
```json
{
  "id": "54032ea5-e21e-4a9b-a02b-488c9922075d",
  "game_session_id": "70b45b09-fe0d-4b98-9e35-62701c9e40c7",
  "name": "Алексей",
  "score": 0
}
```

**Constraints**:
- Можно добавлять игроков только в состоянии `waiting`
- Имя игрока должно быть уникальным в рамках сессии

---

### Game Flow

#### Start Game

Запускает игровую сессию.

**Endpoint**: `POST /games/{game_id}/start`

**Response**: `200 OK`
```json
{
  "id": "70b45b09-fe0d-4b98-9e35-62701c9e40c7",
  "state": "active",
  "current_question_index": 0,
  "message": "Game started successfully"
}
```

**Constraints**:
- Минимум 1 игрок должен быть добавлен
- Игра должна быть в состоянии `waiting`

---

#### Submit Answer

Отправляет ответ игрока на текущий вопрос.

**Endpoint**: `POST /games/{game_id}/answers`

**Request Body** (для multiple_choice):
```json
{
  "player_id": "54032ea5-e21e-4a9b-a02b-488c9922075d",
  "variant_id": "61be4e2f-7892-489c-909b-93b34263c481"
}
```

**Request Body** (для free_text):
```json
{
  "player_id": "54032ea5-e21e-4a9b-a02b-488c9922075d",
  "text_answer": "Москва"
}
```

**Response**: `200 OK`
```json
{
  "is_correct": true,
  "message": "Correct answer!",
  "player_score": 1
}
```

**Possible Responses**:
```json
{
  "is_correct": false,
  "message": "Wrong answer!",
  "player_score": 0
}
```

```json
{
  "is_correct": true,
  "message": "Game finished!",
  "player_score": 3,
  "game_finished": true
}
```

**Constraints**:
- Игра должна быть в состоянии `active`
- Игрок может ответить на вопрос только один раз
- После ответа всех игроков происходит автоматический переход к следующему вопросу

**Game Flow Logic**:
1. Игрок отправляет ответ
2. Система валидирует ответ
3. Обновляется счет игрока (если ответ правильный)
4. Если все игроки ответили:
   - Если это последний вопрос → игра завершается (`state: finished`)
   - Иначе → переход к следующему вопросу (`current_question_index++`)

---

## Error Responses

### Standard Error Format

```json
{
  "error": "Error message description"
}
```

### HTTP Status Codes

- `200 OK` - Успешный запрос
- `400 Bad Request` - Неверные входные данные
- `404 Not Found` - Ресурс не найден
- `500 Internal Server Error` - Внутренняя ошибка сервера

### Common Errors

#### 400 Bad Request
```json
{
  "error": "Invalid UUID format"
}
```

```json
{
  "error": "Missing required field: player_name"
}
```

#### 404 Not Found
```json
{
  "error": "Game session not found"
}
```

```json
{
  "error": "Pack not found"
}
```

#### 500 Internal Server Error
```json
{
  "error": "Failed to create game session"
}
```

```json
{
  "error": "Database connection error"
}
```

---

## Data Models

### Pack
```typescript
interface Pack {
  id: string;           // UUID
  title: string;
}
```

### Question
```typescript
interface Question {
  id: string;           // UUID
  pack_id: string;      // UUID
  text: string;
  image_url: string;    // Optional, can be empty
  question_type: "multiple_choice" | "free_text" | "custom";
}
```

### Variant
```typescript
interface Variant {
  id: string;           // UUID
  question_id: string;  // UUID
  text: string;
  is_correct: boolean;  // Only in admin responses
}
```

### GameSession
```typescript
interface GameSession {
  id: string;                    // UUID
  pack_id: string;               // UUID
  state: "waiting" | "active" | "finished";
  current_question_index: number;
  created_at?: string;           // ISO 8601
  started_at?: string;           // ISO 8601
  finished_at?: string;          // ISO 8601
}
```

### Player
```typescript
interface Player {
  id: string;              // UUID
  game_session_id: string; // UUID
  name: string;
  score: number;
  joined_at?: string;      // ISO 8601
}
```

### PlayerAnswer
```typescript
interface PlayerAnswer {
  id: string;              // UUID
  player_id: string;       // UUID
  question_id: string;     // UUID
  variant_id?: string;     // UUID, for multiple_choice
  text_answer?: string;    // For free_text
  is_correct: boolean;
  answered_at: string;     // ISO 8601
}
```

---

## Rate Limiting

⚠️ **TODO**: Rate limiting не реализован

**Рекомендации**:
- 100 requests/minute per IP для game endpoints
- 1000 requests/minute per IP для content endpoints

---

## Authentication

⚠️ **TODO**: Authentication не реализован

Все endpoints в данный момент публичные.

**Планируется**:
- JWT tokens для игроков
- API keys для администраторов
- OAuth2 для интеграций

---

## Versioning

**Current Version**: v1 (implicit)

**Future**: API versioning через URL prefix:
- `/v1/games`
- `/v2/games`

---

## WebSocket API (Planned)

⚠️ **TODO**: WebSocket не реализован

**Планируемые события**:
```typescript
// Client → Server
{
  "type": "join_game",
  "game_id": "uuid",
  "player_id": "uuid"
}

// Server → Client
{
  "type": "player_joined",
  "player": { ... }
}

{
  "type": "question_changed",
  "question": { ... },
  "variants": [ ... ]
}

{
  "type": "player_answered",
  "player_id": "uuid",
  "is_correct": boolean
}

{
  "type": "game_finished",
  "results": { ... }
}
```

---

## Examples

См. [`GAME_FLOW.md`](GAME_FLOW.md) для полных примеров использования API.

---

## Support

Для вопросов и багрепортов создавайте issues в репозитории проекта.