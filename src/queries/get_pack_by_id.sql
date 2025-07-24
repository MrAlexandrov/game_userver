SELECT
    id AS pack_id,
    title
FROM quiz.packs
WHERE id = $1;
