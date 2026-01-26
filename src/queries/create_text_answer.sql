INSERT INTO quiz.text_answers (question_id, text)
VALUES ($1, $2)
RETURNING id, question_id, text, created_at;
