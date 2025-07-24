INSERT INTO quiz.questions (pack_id, text, image_url)
VALUES ($1, $2, $3)
RETURNING id, pack_id, text, image_url;
