SELECT id, question_id, text, is_correct
FROM quiz.variants
WHERE question_id = $1;
