INSERT INTO quiz.game_sessions (pack_id)
VALUES ($1)
RETURNING id, pack_id, state, current_question_index, created_at, started_at, finished_at;