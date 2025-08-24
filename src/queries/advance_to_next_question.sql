UPDATE quiz.game_sessions
SET current_question_index = current_question_index + 1
WHERE id = $1
RETURNING id, pack_id, state, current_question_index, created_at, started_at, finished_at;