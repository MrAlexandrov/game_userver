INSERT INTO quiz.questions (pack_id, text, image_url, question_type)
VALUES ($1, $2, $3, $4)
RETURNING id, pack_id, text, image_url, question_type;
