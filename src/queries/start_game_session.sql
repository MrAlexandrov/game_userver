UPDATE quiz.game_sessions
SET state = 'active', started_at = NOW()
WHERE id = $1
RETURNING id, pack_id, state, current_question_index, created_at, started_at, finished_at;