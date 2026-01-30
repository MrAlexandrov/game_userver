UPDATE quiz.players
SET score = score + $2
WHERE id = $1
RETURNING id, game_session_id, name, score, joined_at;
