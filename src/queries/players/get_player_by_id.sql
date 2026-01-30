SELECT id, game_session_id, name, score, joined_at
FROM quiz.players
WHERE id = $1;