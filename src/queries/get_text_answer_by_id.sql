SELECT id, question_id, text, created_at
FROM quiz.text_answers
WHERE id = $1;
