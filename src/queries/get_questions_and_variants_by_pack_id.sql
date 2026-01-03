SELECT 
    q.id AS question_id,
    q.pack_id AS question_pack_id,
    q.text AS question_text,
    q.image_url AS question_image_url,
    v.id AS variant_id,
    v.question_id AS variant_question_id,
    v.text AS variant_text,
    v.is_correct AS variant_is_correct
FROM quiz.questions q
LEFT JOIN quiz.variants v ON q.id = v.question_id
WHERE q.pack_id = $1
ORDER BY q.id, v.id;