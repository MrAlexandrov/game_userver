INSERT INTO quiz.packs (title)
VALUES ($1)
RETURNING id AS pack_id, title;
