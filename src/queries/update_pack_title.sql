UPDATE quiz.packs
SET title = $2
WHERE id = $1
RETURNING id, title;
