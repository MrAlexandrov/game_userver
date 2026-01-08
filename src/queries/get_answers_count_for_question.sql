SELECT COUNT(DISTINCT pa.player_id) as count
FROM quiz.player_answers pa
JOIN quiz.players p ON pa.player_id = p.id
WHERE p.game_session_id = $1 AND pa.question_id = $2;
