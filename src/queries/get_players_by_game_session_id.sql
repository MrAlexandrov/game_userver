SELECT id, game_session_id, name, score, joined_at
FROM quiz.players
WHERE game_session_id = $1
ORDER BY joined_at;
