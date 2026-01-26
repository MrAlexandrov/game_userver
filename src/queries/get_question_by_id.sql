SELECT id, pack_id, text, image_url, question_type
FROM quiz.questions
WHERE id = $1;
