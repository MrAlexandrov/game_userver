SELECT id, player_id, question_id, variant_id, is_correct, answered_at
FROM quiz.player_answers
WHERE player_id = $1
ORDER BY answered_at ASC;
