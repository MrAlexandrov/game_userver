SELECT id, question_id, text, is_correct
FROM quiz.variants
WHERE id = $1;
