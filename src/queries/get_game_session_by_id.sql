SELECT id, pack_id, state, current_question_index, created_at, started_at, finished_at
FROM quiz.game_sessions
WHERE id = $1;