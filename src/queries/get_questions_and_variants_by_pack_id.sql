SELECT
    q.id AS question_id,
    q.pack_id,
    q.text AS question_text,
    q.image_url,
    jsonb_agg(jsonb_build_object(
        'id', v.id,
        'text', v.text,
        'is_correct', v.is_correct
    )) AS variants
FROM quiz.questions q
JOIN quiz.variants v ON q.id = v.question_id
WHERE q.pack_id = $1
GROUP BY q.id;