INSERT INTO quiz.player_answers (player_id, question_id, variant_id, text_answer, is_correct)
VALUES ($1, $2, $3, $4, $5)
RETURNING id, player_id, question_id, variant_id, text_answer, is_correct, answered_at;
