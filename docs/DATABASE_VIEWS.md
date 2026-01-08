# SQL Views для анализа данных игры

Этот документ описывает SQL views, созданные для удобного анализа данных в человекочитаемом формате.

## Обзор

Вместо работы с UUID и JOIN'ами вручную, вы можете использовать готовые views, которые предоставляют данные в удобном формате с названиями, статистикой и вычисляемыми полями.

## Применение views

Чтобы применить views к вашей базе данных, выполните:

```bash
psql -U your_user -d your_database -f postgresql/schemas/views.sql
```

Или через Docker:

```bash
docker exec -i postgres_container psql -U user -d dbname < postgresql/schemas/views.sql
```

## Доступные Views

### 1. game_sessions_view - Игровые сессии

Полная информация об игровых сессиях с названием пака и статистикой.

**Поля:**
- `game_session_id` - ID сессии
- `state` - Состояние (waiting/active/finished)
- `current_question_index` - Текущий вопрос
- `created_at`, `started_at`, `finished_at` - Временные метки
- `pack_id`, `pack_title` - ID и название пака
- `duration_seconds` - Длительность игры в секундах
- `total_players` - Количество игроков
- `total_questions` - Количество вопросов

**Пример использования:**
```sql
-- Все игровые сессии
SELECT * FROM quiz.game_sessions_view;

-- Активные игры
SELECT * FROM quiz.game_sessions_view WHERE state = 'active';

-- Игры по конкретному паку
SELECT * FROM quiz.game_sessions_view WHERE pack_title = 'Викторина по географии';

-- Самые длинные игры
SELECT game_session_id, pack_title, duration_seconds 
FROM quiz.game_sessions_view 
WHERE state = 'finished'
ORDER BY duration_seconds DESC 
LIMIT 10;
```

### 2. players_view - Игроки с статистикой

Игроки с полной статистикой и информацией об игровой сессии.

**Поля:**
- `player_id`, `player_name` - ID и имя игрока
- `score` - Счёт
- `joined_at` - Время присоединения
- `game_session_id`, `game_state` - Информация о сессии
- `pack_title` - Название пака
- `total_answers` - Всего ответов
- `correct_answers` - Правильных ответов
- `incorrect_answers` - Неправильных ответов
- `accuracy_percentage` - Точность в процентах

**Пример использования:**
```sql
-- Все игроки
SELECT * FROM quiz.players_view;

-- Игроки с точностью выше 80%
SELECT player_name, score, accuracy_percentage 
FROM quiz.players_view 
WHERE accuracy_percentage > 80
ORDER BY accuracy_percentage DESC;

-- Игроки конкретной игры
SELECT player_name, score, correct_answers, accuracy_percentage
FROM quiz.players_view 
WHERE game_session_id = 'your-game-id'
ORDER BY score DESC;
```

### 3. player_answers_view - Ответы игроков

Полная информация об ответах с текстами вопросов и вариантов.

**Поля:**
- `answer_id`, `answered_at`, `is_correct` - Информация об ответе
- `player_id`, `player_name`, `player_score` - Информация об игроке
- `game_session_id`, `game_state` - Информация о сессии
- `pack_id`, `pack_title` - Информация о паке
- `question_id`, `question_text`, `question_image_url` - Вопрос
- `variant_id`, `variant_text`, `variant_is_correct` - Выбранный вариант
- `correct_answer_text` - Правильный ответ

**Пример использования:**
```sql
-- Последние 10 ответов
SELECT player_name, question_text, variant_text, is_correct, answered_at
FROM quiz.player_answers_view 
ORDER BY answered_at DESC 
LIMIT 10;

-- Неправильные ответы игрока
SELECT question_text, variant_text, correct_answer_text
FROM quiz.player_answers_view 
WHERE player_name = 'Алексей' AND is_correct = false;

-- Ответы на конкретный вопрос
SELECT player_name, variant_text, is_correct
FROM quiz.player_answers_view 
WHERE question_text = 'Столица России'
ORDER BY answered_at;
```

### 4. questions_with_variants_view - Вопросы с вариантами

Вопросы со всеми вариантами ответов и статистикой использования.

**Поля:**
- `question_id`, `question_text`, `question_image_url` - Вопрос
- `pack_id`, `pack_title` - Пак
- `variant_id`, `variant_text`, `is_correct_answer` - Вариант
- `times_answered` - Сколько раз отвечали на вопрос
- `times_this_variant_chosen` - Сколько раз выбирали этот вариант

**Пример использования:**
```sql
-- Все вопросы с вариантами
SELECT * FROM quiz.questions_with_variants_view;

-- Вопросы конкретного пака
SELECT question_text, variant_text, is_correct_answer
FROM quiz.questions_with_variants_view 
WHERE pack_title = 'Викторина по географии';

-- Самые популярные неправильные ответы
SELECT question_text, variant_text, times_this_variant_chosen
FROM quiz.questions_with_variants_view 
WHERE is_correct_answer = false
ORDER BY times_this_variant_chosen DESC 
LIMIT 10;
```

### 5. packs_statistics_view - Статистика по пакам

Полная статистика по каждому паку.

**Поля:**
- `pack_id`, `pack_title` - Пак
- `total_questions`, `total_variants` - Количество вопросов и вариантов
- `total_games` - Всего игр
- `games_waiting`, `games_active`, `games_finished` - По статусам
- `total_unique_players` - Уникальных игроков
- `average_accuracy_percentage` - Средняя точность ответов

**Пример использования:**
```sql
-- Статистика по всем пакам
SELECT * FROM quiz.packs_statistics_view;

-- Самые популярные паки
SELECT pack_title, total_games, total_unique_players
FROM quiz.packs_statistics_view 
ORDER BY total_games DESC;

-- Самые сложные паки (низкая точность)
SELECT pack_title, average_accuracy_percentage
FROM quiz.packs_statistics_view 
WHERE total_games > 0
ORDER BY average_accuracy_percentage ASC;
```

### 6. game_leaderboard_view - Рейтинг игроков

Рейтинг игроков по каждой игровой сессии.

**Поля:**
- `game_session_id`, `game_state`, `pack_title` - Игра
- `player_id`, `player_name` - Игрок
- `score` - Счёт
- `total_answers`, `correct_answers` - Статистика ответов
- `rank` - Место в рейтинге
- `accuracy_percentage` - Точность

**Пример использования:**
```sql
-- Рейтинг конкретной игры
SELECT rank, player_name, score, accuracy_percentage
FROM quiz.game_leaderboard_view 
WHERE game_session_id = 'your-game-id'
ORDER BY rank;

-- Топ-3 игроков каждой игры
SELECT game_session_id, pack_title, player_name, score, rank
FROM quiz.game_leaderboard_view 
WHERE rank <= 3
ORDER BY game_session_id, rank;
```

### 7. active_games_view - Активные игры

Активные и ожидающие игры с информацией об игроках и текущем вопросе.

**Поля:**
- `game_session_id`, `state` - Игра
- `current_question_index` - Текущий вопрос
- `started_at` - Время старта
- `pack_title` - Название пака
- `total_questions`, `total_players` - Количество
- `player_names` - Имена игроков (через запятую)
- `current_question_text` - Текст текущего вопроса
- `players_answered_current_question` - Сколько ответили

**Пример использования:**
```sql
-- Все активные игры
SELECT * FROM quiz.active_games_view;

-- Игры, ожидающие игроков
SELECT game_session_id, pack_title, total_players, player_names
FROM quiz.active_games_view 
WHERE state = 'waiting';

-- Активные игры с прогрессом
SELECT pack_title, current_question_index, total_questions,
       players_answered_current_question, total_players
FROM quiz.active_games_view 
WHERE state = 'active';
```

### 8. questions_statistics_view - Статистика по вопросам

Статистика по каждому вопросу: количество ответов, точность, популярные ошибки.

**Поля:**
- `question_id`, `question_text`, `question_image_url` - Вопрос
- `pack_id`, `pack_title` - Пак
- `total_answers` - Всего ответов
- `correct_answers`, `incorrect_answers` - Правильных/неправильных
- `correct_percentage` - Процент правильных
- `most_common_wrong_answer` - Самая популярная ошибка
- `correct_answer` - Правильный ответ

**Пример использования:**
```sql
-- Статистика по всем вопросам
SELECT * FROM quiz.questions_statistics_view;

-- Самые сложные вопросы
SELECT question_text, correct_percentage, most_common_wrong_answer
FROM quiz.questions_statistics_view 
WHERE total_answers > 0
ORDER BY correct_percentage ASC 
LIMIT 10;

-- Самые лёгкие вопросы
SELECT question_text, correct_percentage
FROM quiz.questions_statistics_view 
WHERE total_answers > 0
ORDER BY correct_percentage DESC 
LIMIT 10;
```

### 9. game_history_view - История игр

История завершённых игр с результатами и статистикой.

**Поля:**
- `game_session_id`, `state` - Игра
- `created_at`, `started_at`, `finished_at` - Временные метки
- `pack_title` - Название пака
- `total_questions`, `total_players` - Количество
- `winner_name`, `winner_score` - Победитель
- `average_score` - Средний счёт
- `average_accuracy` - Средняя точность
- `duration_seconds` - Длительность

**Пример использования:**
```sql
-- История всех игр
SELECT * FROM quiz.game_history_view;

-- Последние 10 игр
SELECT pack_title, winner_name, winner_score, finished_at
FROM quiz.game_history_view 
ORDER BY finished_at DESC 
LIMIT 10;

-- Игры с высокой средней точностью
SELECT pack_title, average_accuracy, total_players
FROM quiz.game_history_view 
WHERE average_accuracy > 80
ORDER BY average_accuracy DESC;
```

### 10. top_players_view - Топ игроков

Общий рейтинг игроков по всем играм.

**Поля:**
- `player_name` - Имя игрока
- `games_played` - Сыграно игр
- `total_score` - Общий счёт
- `average_score` - Средний счёт
- `total_answers` - Всего ответов
- `correct_answers` - Правильных ответов
- `accuracy_percentage` - Точность
- `wins` - Количество побед

**Пример использования:**
```sql
-- Топ-10 игроков
SELECT * FROM quiz.top_players_view LIMIT 10;

-- Игроки с наибольшим количеством побед
SELECT player_name, wins, games_played, accuracy_percentage
FROM quiz.top_players_view 
ORDER BY wins DESC 
LIMIT 10;

-- Самые точные игроки (минимум 10 игр)
SELECT player_name, accuracy_percentage, games_played
FROM quiz.top_players_view 
WHERE games_played >= 10
ORDER BY accuracy_percentage DESC 
LIMIT 10;
```

## Примеры аналитических запросов

### Анализ сложности вопросов

```sql
-- Вопросы, на которые чаще всего отвечают неправильно
SELECT 
    pack_title,
    question_text,
    correct_percentage,
    most_common_wrong_answer
FROM quiz.questions_statistics_view
WHERE total_answers >= 5
ORDER BY correct_percentage ASC
LIMIT 20;
```

### Анализ активности игроков

```sql
-- Самые активные игроки за последнюю неделю
SELECT 
    pv.player_name,
    COUNT(DISTINCT pv.game_session_id) as games_this_week,
    AVG(pv.accuracy_percentage) as avg_accuracy
FROM quiz.players_view pv
WHERE pv.joined_at >= NOW() - INTERVAL '7 days'
GROUP BY pv.player_name
ORDER BY games_this_week DESC
LIMIT 10;
```

### Анализ популярности паков

```sql
-- Популярность паков по времени суток
SELECT 
    p.title as pack_title,
    EXTRACT(HOUR FROM gs.started_at) as hour_of_day,
    COUNT(*) as games_count
FROM quiz.game_sessions gs
JOIN quiz.packs p ON gs.pack_id = p.id
WHERE gs.started_at IS NOT NULL
GROUP BY p.title, EXTRACT(HOUR FROM gs.started_at)
ORDER BY p.title, hour_of_day;
```

### Сравнение игроков

```sql
-- Сравнение двух игроков
SELECT 
    player_name,
    games_played,
    total_score,
    average_score,
    accuracy_percentage,
    wins
FROM quiz.top_players_view
WHERE player_name IN ('Алексей', 'Мария');
```

## Производительность

Все views оптимизированы и используют существующие индексы. Для больших объёмов данных рекомендуется:

1. Использовать фильтрацию по датам
2. Ограничивать результаты с помощью LIMIT
3. Создать материализованные views для часто используемых запросов

### Создание материализованного view

```sql
-- Пример материализованного view для статистики
CREATE MATERIALIZED VIEW quiz.packs_statistics_materialized AS
SELECT * FROM quiz.packs_statistics_view;

-- Обновление материализованного view
REFRESH MATERIALIZED VIEW quiz.packs_statistics_materialized;

-- Автоматическое обновление (через cron или pg_cron)
-- Каждый час:
SELECT cron.schedule('refresh-stats', '0 * * * *', 
    'REFRESH MATERIALIZED VIEW quiz.packs_statistics_materialized');
```

## Интеграция с приложением

Views можно использовать напрямую из C++ кода:

```cpp
// Пример запроса к view
auto result = pg_cluster->Execute(
    userver::storages::postgres::ClusterHostType::kSlave,
    "SELECT * FROM quiz.game_leaderboard_view WHERE game_session_id = $1",
    game_session_id
);
```

## Экспорт данных

### Экспорт в CSV

```bash
# Экспорт топ игроков
psql -U user -d dbname -c "COPY (SELECT * FROM quiz.top_players_view) TO STDOUT WITH CSV HEADER" > top_players.csv

# Экспорт истории игр
psql -U user -d dbname -c "COPY (SELECT * FROM quiz.game_history_view) TO STDOUT WITH CSV HEADER" > game_history.csv
```

### Экспорт в JSON

```sql
-- Экспорт в JSON
SELECT json_agg(row_to_json(t)) 
FROM (SELECT * FROM quiz.top_players_view LIMIT 10) t;
```

## Мониторинг

Используйте views для создания дашбордов в Grafana, Metabase или других инструментах визуализации.

### Пример метрик для мониторинга

```sql
-- Активные игры прямо сейчас
SELECT COUNT(*) FROM quiz.active_games_view WHERE state = 'active';

-- Игроков онлайн
SELECT SUM(total_players) FROM quiz.active_games_view WHERE state = 'active';

-- Средняя длительность игры за последний день
SELECT AVG(duration_seconds) 
FROM quiz.game_history_view 
WHERE finished_at >= NOW() - INTERVAL '1 day';
```

## Заключение

SQL views предоставляют удобный способ анализа данных без необходимости писать сложные JOIN'ы. Все данные представлены в человекочитаемом формате с названиями вместо UUID.

## См. также

- [GAME_FLOW.md](GAME_FLOW.md) - Процесс игры
- [OBSERVER_PATTERN.md](OBSERVER_PATTERN.md) - Система событий
- Схема базы данных: `postgresql/schemas/db_1.sql`
- Views: `postgresql/schemas/views.sql`