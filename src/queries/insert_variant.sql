INSERT INTO quiz.variants (question_id, text, is_correct)
VALUES ($1, $2, $3)
RETURNING id, question_id, text, is_correct;
