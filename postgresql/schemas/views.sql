-- ============================================================================
-- SQL Views для удобного анализа данных игры
-- ============================================================================

-- View: Полная информация об игровых сессиях с названием пака
CREATE OR REPLACE VIEW quiz.game_sessions_view AS
SELECT 
    gs.id AS game_session_id,
    gs.state,
    gs.current_question_index,
    gs.created_at,
    gs.started_at,
    gs.finished_at,
    p.id AS pack_id,
    p.title AS pack_title,
    -- Вычисляемые поля
    CASE 
        WHEN gs.finished_at IS NOT NULL THEN 
            EXTRACT(EPOCH FROM (gs.finished_at - gs.started_at))
        WHEN gs.started_at IS NOT NULL THEN 
            EXTRACT(EPOCH FROM (NOW() - gs.started_at))
        ELSE NULL
    END AS duration_seconds,
    (SELECT COUNT(*) FROM quiz.players WHERE game_session_id = gs.id) AS total_players,
    (SELECT COUNT(*) FROM quiz.questions WHERE pack_id = gs.pack_id) AS total_questions
FROM quiz.game_sessions gs
JOIN quiz.packs p ON gs.pack_id = p.id
ORDER BY gs.created_at DESC;

COMMENT ON VIEW quiz.game_sessions_view IS 
'Полная информация об игровых сессиях с названием пака и статистикой';


-- View: Игроки с информацией об игровой сессии
CREATE OR REPLACE VIEW quiz.players_view AS
SELECT 
    pl.id AS player_id,
    pl.name AS player_name,
    pl.score,
    pl.joined_at,
    gs.id AS game_session_id,
    gs.state AS game_state,
    p.title AS pack_title,
    -- Вычисляемые поля
    (SELECT COUNT(*) FROM quiz.player_answers WHERE player_id = pl.id) AS total_answers,
    (SELECT COUNT(*) FROM quiz.player_answers WHERE player_id = pl.id AND is_correct = true) AS correct_answers,
    (SELECT COUNT(*) FROM quiz.player_answers WHERE player_id = pl.id AND is_correct = false) AS incorrect_answers,
    CASE 
        WHEN (SELECT COUNT(*) FROM quiz.player_answers WHERE player_id = pl.id) > 0 THEN
            ROUND(
                (SELECT COUNT(*)::NUMERIC FROM quiz.player_answers WHERE player_id = pl.id AND is_correct = true) * 100.0 / 
                (SELECT COUNT(*) FROM quiz.player_answers WHERE player_id = pl.id),
                2
            )
        ELSE 0
    END AS accuracy_percentage
FROM quiz.players pl
JOIN quiz.game_sessions gs ON pl.game_session_id = gs.id
JOIN quiz.packs p ON gs.pack_id = p.id
ORDER BY pl.joined_at DESC;

COMMENT ON VIEW quiz.players_view IS 
'Игроки с полной статистикой и информацией об игровой сессии';


-- View: Ответы игроков с полной информацией
CREATE OR REPLACE VIEW quiz.player_answers_view AS
SELECT 
    pa.id AS answer_id,
    pa.answered_at,
    pa.is_correct,
    -- Информация об игроке
    pl.id AS player_id,
    pl.name AS player_name,
    pl.score AS player_score,
    -- Информация об игровой сессии
    gs.id AS game_session_id,
    gs.state AS game_state,
    -- Информация о паке
    p.id AS pack_id,
    p.title AS pack_title,
    -- Информация о вопросе
    q.id AS question_id,
    q.text AS question_text,
    q.image_url AS question_image_url,
    -- Информация о выбранном варианте
    v.id AS variant_id,
    v.text AS variant_text,
    v.is_correct AS variant_is_correct,
    -- Правильный ответ
    (SELECT text FROM quiz.variants WHERE question_id = q.id AND is_correct = true LIMIT 1) AS correct_answer_text
FROM quiz.player_answers pa
JOIN quiz.players pl ON pa.player_id = pl.id
JOIN quiz.game_sessions gs ON pl.game_session_id = gs.id
JOIN quiz.packs p ON gs.pack_id = p.id
JOIN quiz.questions q ON pa.question_id = q.id
JOIN quiz.variants v ON pa.variant_id = v.id
ORDER BY pa.answered_at DESC;

COMMENT ON VIEW quiz.player_answers_view IS 
'Полная информация об ответах игроков с текстами вопросов и вариантов';


-- View: Вопросы с вариантами ответов
CREATE OR REPLACE VIEW quiz.questions_with_variants_view AS
SELECT 
    q.id AS question_id,
    q.text AS question_text,
    q.image_url AS question_image_url,
    p.id AS pack_id,
    p.title AS pack_title,
    v.id AS variant_id,
    v.text AS variant_text,
    v.is_correct AS is_correct_answer,
    -- Статистика по вопросу
    (SELECT COUNT(*) FROM quiz.player_answers WHERE question_id = q.id) AS times_answered,
    (SELECT COUNT(*) FROM quiz.player_answers WHERE question_id = q.id AND variant_id = v.id) AS times_this_variant_chosen
FROM quiz.questions q
JOIN quiz.packs p ON q.pack_id = p.id
JOIN quiz.variants v ON v.question_id = q.id
ORDER BY p.title, q.id, v.is_correct DESC, v.text;

COMMENT ON VIEW quiz.questions_with_variants_view IS 
'Вопросы со всеми вариантами ответов и статистикой использования';


-- View: Статистика по пакам
CREATE OR REPLACE VIEW quiz.packs_statistics_view AS
SELECT 
    p.id AS pack_id,
    p.title AS pack_title,
    -- Количество вопросов
    (SELECT COUNT(*) FROM quiz.questions WHERE pack_id = p.id) AS total_questions,
    -- Количество вариантов
    (SELECT COUNT(*) FROM quiz.variants v 
     JOIN quiz.questions q ON v.question_id = q.id 
     WHERE q.pack_id = p.id) AS total_variants,
    -- Статистика игр
    (SELECT COUNT(*) FROM quiz.game_sessions WHERE pack_id = p.id) AS total_games,
    (SELECT COUNT(*) FROM quiz.game_sessions WHERE pack_id = p.id AND state = 'waiting') AS games_waiting,
    (SELECT COUNT(*) FROM quiz.game_sessions WHERE pack_id = p.id AND state = 'active') AS games_active,
    (SELECT COUNT(*) FROM quiz.game_sessions WHERE pack_id = p.id AND state = 'finished') AS games_finished,
    -- Статистика игроков
    (SELECT COUNT(DISTINCT pl.id) FROM quiz.players pl 
     JOIN quiz.game_sessions gs ON pl.game_session_id = gs.id 
     WHERE gs.pack_id = p.id) AS total_unique_players,
    -- Средняя точность ответов
    (SELECT ROUND(AVG(CASE WHEN pa.is_correct THEN 100.0 ELSE 0.0 END), 2)
     FROM quiz.player_answers pa
     JOIN quiz.questions q ON pa.question_id = q.id
     WHERE q.pack_id = p.id) AS average_accuracy_percentage
FROM quiz.packs p
ORDER BY p.title;

COMMENT ON VIEW quiz.packs_statistics_view IS 
'Статистика по пакам: вопросы, игры, игроки, точность ответов';


-- View: Рейтинг игроков по игровой сессии
CREATE OR REPLACE VIEW quiz.game_leaderboard_view AS
SELECT 
    pl.game_session_id,
    gs.state AS game_state,
    p.title AS pack_title,
    pl.id AS player_id,
    pl.name AS player_name,
    pl.score,
    (SELECT COUNT(*) FROM quiz.player_answers WHERE player_id = pl.id) AS total_answers,
    (SELECT COUNT(*) FROM quiz.player_answers WHERE player_id = pl.id AND is_correct = true) AS correct_answers,
    RANK() OVER (PARTITION BY pl.game_session_id ORDER BY pl.score DESC, pl.joined_at ASC) AS rank,
    CASE 
        WHEN (SELECT COUNT(*) FROM quiz.player_answers WHERE player_id = pl.id) > 0 THEN
            ROUND(
                (SELECT COUNT(*)::NUMERIC FROM quiz.player_answers WHERE player_id = pl.id AND is_correct = true) * 100.0 / 
                (SELECT COUNT(*) FROM quiz.player_answers WHERE player_id = pl.id),
                2
            )
        ELSE 0
    END AS accuracy_percentage
FROM quiz.players pl
JOIN quiz.game_sessions gs ON pl.game_session_id = gs.id
JOIN quiz.packs p ON gs.pack_id = p.id
ORDER BY pl.game_session_id, rank;

COMMENT ON VIEW quiz.game_leaderboard_view IS 
'Рейтинг игроков по каждой игровой сессии';


-- View: Активные игры с игроками
CREATE OR REPLACE VIEW quiz.active_games_view AS
SELECT 
    gs.id AS game_session_id,
    gs.state,
    gs.current_question_index,
    gs.started_at,
    p.title AS pack_title,
    (SELECT COUNT(*) FROM quiz.questions WHERE pack_id = gs.pack_id) AS total_questions,
    (SELECT COUNT(*) FROM quiz.players WHERE game_session_id = gs.id) AS total_players,
    STRING_AGG(pl.name, ', ' ORDER BY pl.joined_at) AS player_names,
    -- Текущий вопрос
    (SELECT text FROM quiz.questions 
     WHERE pack_id = gs.pack_id 
     ORDER BY id 
     OFFSET gs.current_question_index 
     LIMIT 1) AS current_question_text,
    -- Сколько игроков ответили на текущий вопрос
    (SELECT COUNT(DISTINCT pa.player_id) 
     FROM quiz.player_answers pa
     JOIN quiz.questions q ON pa.question_id = q.id
     WHERE q.pack_id = gs.pack_id 
     AND pa.player_id IN (SELECT id FROM quiz.players WHERE game_session_id = gs.id)
     AND q.id = (SELECT id FROM quiz.questions 
                 WHERE pack_id = gs.pack_id 
                 ORDER BY id 
                 OFFSET gs.current_question_index 
                 LIMIT 1)
    ) AS players_answered_current_question
FROM quiz.game_sessions gs
JOIN quiz.packs p ON gs.pack_id = p.id
LEFT JOIN quiz.players pl ON pl.game_session_id = gs.id
WHERE gs.state IN ('waiting', 'active')
GROUP BY gs.id, gs.state, gs.current_question_index, gs.started_at, p.title, gs.pack_id
ORDER BY gs.started_at DESC NULLS LAST;

COMMENT ON VIEW quiz.active_games_view IS 
'Активные и ожидающие игры с информацией об игроках и текущем вопросе';


-- View: Статистика по вопросам
CREATE OR REPLACE VIEW quiz.questions_statistics_view AS
SELECT 
    q.id AS question_id,
    q.text AS question_text,
    q.image_url AS question_image_url,
    p.id AS pack_id,
    p.title AS pack_title,
    -- Статистика ответов
    (SELECT COUNT(*) FROM quiz.player_answers WHERE question_id = q.id) AS total_answers,
    (SELECT COUNT(*) FROM quiz.player_answers WHERE question_id = q.id AND is_correct = true) AS correct_answers,
    (SELECT COUNT(*) FROM quiz.player_answers WHERE question_id = q.id AND is_correct = false) AS incorrect_answers,
    -- Процент правильных ответов
    CASE 
        WHEN (SELECT COUNT(*) FROM quiz.player_answers WHERE question_id = q.id) > 0 THEN
            ROUND(
                (SELECT COUNT(*)::NUMERIC FROM quiz.player_answers WHERE question_id = q.id AND is_correct = true) * 100.0 / 
                (SELECT COUNT(*) FROM quiz.player_answers WHERE question_id = q.id),
                2
            )
        ELSE NULL
    END AS correct_percentage,
    -- Самый популярный неправильный ответ
    (SELECT v.text 
     FROM quiz.player_answers pa
     JOIN quiz.variants v ON pa.variant_id = v.id
     WHERE pa.question_id = q.id AND pa.is_correct = false
     GROUP BY v.text
     ORDER BY COUNT(*) DESC
     LIMIT 1) AS most_common_wrong_answer,
    -- Правильный ответ
    (SELECT text FROM quiz.variants WHERE question_id = q.id AND is_correct = true LIMIT 1) AS correct_answer
FROM quiz.questions q
JOIN quiz.packs p ON q.pack_id = p.id
ORDER BY p.title, q.id;

COMMENT ON VIEW quiz.questions_statistics_view IS 
'Статистика по вопросам: количество ответов, точность, популярные ошибки';


-- View: История игр с результатами
CREATE OR REPLACE VIEW quiz.game_history_view AS
SELECT 
    gs.id AS game_session_id,
    gs.state,
    gs.created_at,
    gs.started_at,
    gs.finished_at,
    p.title AS pack_title,
    (SELECT COUNT(*) FROM quiz.questions WHERE pack_id = gs.pack_id) AS total_questions,
    (SELECT COUNT(*) FROM quiz.players WHERE game_session_id = gs.id) AS total_players,
    -- Победитель
    (SELECT name FROM quiz.players 
     WHERE game_session_id = gs.id 
     ORDER BY score DESC, joined_at ASC 
     LIMIT 1) AS winner_name,
    (SELECT score FROM quiz.players 
     WHERE game_session_id = gs.id 
     ORDER BY score DESC, joined_at ASC 
     LIMIT 1) AS winner_score,
    -- Средний счёт
    (SELECT ROUND(AVG(score), 2) FROM quiz.players WHERE game_session_id = gs.id) AS average_score,
    -- Общая точность
    (SELECT ROUND(AVG(CASE WHEN pa.is_correct THEN 100.0 ELSE 0.0 END), 2)
     FROM quiz.player_answers pa
     JOIN quiz.players pl ON pa.player_id = pl.id
     WHERE pl.game_session_id = gs.id) AS average_accuracy,
    -- Длительность игры
    CASE 
        WHEN gs.finished_at IS NOT NULL AND gs.started_at IS NOT NULL THEN 
            EXTRACT(EPOCH FROM (gs.finished_at - gs.started_at))
        ELSE NULL
    END AS duration_seconds
FROM quiz.game_sessions gs
JOIN quiz.packs p ON gs.pack_id = p.id
WHERE gs.state = 'finished'
ORDER BY gs.finished_at DESC;

COMMENT ON VIEW quiz.game_history_view IS 
'История завершённых игр с результатами и статистикой';


-- -- View: Топ игроков (общий рейтинг)
-- CREATE OR REPLACE VIEW quiz.top_players_view AS
-- SELECT 
--     pl.name AS player_name,
--     COUNT(DISTINCT pl.game_session_id) AS games_played,
--     SUM(pl.score) AS total_score,
--     ROUND(AVG(pl.score), 2) AS average_score,
--     (SELECT COUNT(*) FROM quiz.player_answers pa WHERE pa.player_id = pl.id) AS total_answers,
--     (SELECT COUNT(*) FROM quiz.player_answers pa WHERE pa.player_id = pl.id AND pa.is_correct = true) AS correct_answers,
--     CASE 
--         WHEN (SELECT COUNT(*) FROM quiz.player_answers pa WHERE pa.player_id = pl.id) > 0 THEN
--             ROUND(
--                 (SELECT COUNT(*)::NUMERIC FROM quiz.player_answers pa WHERE pa.player_id = pl.id AND pa.is_correct = true) * 100.0 / 
--                 (SELECT COUNT(*) FROM quiz.player_answers pa WHERE pa.player_id = pl.id),
--                 2
--             )
--         ELSE 0
--     END AS accuracy_percentage,
--     -- Количество побед
--     (SELECT COUNT(*) FROM (
--         SELECT game_session_id, MAX(score) as max_score
--         FROM quiz.players
--         GROUP BY game_session_id
--     ) AS max_scores
--     WHERE max_scores.game_session_id IN (SELECT game_session_id FROM quiz.players WHERE name = pl.name)
--     AND max_scores.max_score = (SELECT score FROM quiz.players WHERE game_session_id = max_scores.game_session_id AND name = pl.name)
--     ) AS wins
-- FROM quiz.players pl
-- GROUP BY pl.name
-- ORDER BY total_score DESC, accuracy_percentage DESC;

COMMENT ON VIEW quiz.top_players_view IS 
'Общий рейтинг игроков по всем играм';
