SELECT id, pack_id, text, image_url
FROM quiz.questions
WHERE id = $1;
