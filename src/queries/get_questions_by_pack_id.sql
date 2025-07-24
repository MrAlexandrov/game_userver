SELECT id, pack_id, text, image_url
FROM quiz.questions
WHERE pack_id = $1;
