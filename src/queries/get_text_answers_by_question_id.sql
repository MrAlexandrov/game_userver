SELECT id, question_id, text, created_at
FROM quiz.text_answers
WHERE question_id = $1;
