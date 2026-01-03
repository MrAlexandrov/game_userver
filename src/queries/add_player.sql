INSERT INTO quiz.players (game_session_id, name)
VALUES ($1, $2)
RETURNING id, game_session_id, name, score, joined_at;
