import pytest
from endpoints import (
    create_pack,

    create_question,

    create_variant,
    get_variant_by_id,
    get_variants_by_question_id,
)


@pytest.fixture
async def sample_questions(service_client):
    questions = []
    pack = await create_pack(service_client, 'Pack')
    for i in range(2):
        question = await create_question(service_client, pack["id"], f"Question {i}", f"image_url_{i}")
        questions.append(question)

    return questions


async def test_create_variant(service_client, sample_questions):
    await create_variant(service_client, sample_questions[0]["id"], "some_text", True)
    await create_variant(service_client, sample_questions[0]["id"], "some_text", False)
    await create_variant(service_client, sample_questions[0]["id"], "some_text")


async def test_get_variant_by_id(service_client, sample_questions):
    created_variant = await create_variant(service_client, sample_questions[0]["id"], "some_text", True)
    getted_variant = await get_variant_by_id(service_client, created_variant["id"])

    assert created_variant == getted_variant


async def test_get_variants_by_question_id(service_client, sample_questions):
    first_variant = await create_variant(service_client, sample_questions[0]["id"], "first_variant", True)
    second_variant = await create_variant(service_client, sample_questions[1]["id"], "second_variant")

    variants_from_first_question = await get_variants_by_question_id(service_client, sample_questions[0]["id"])
    variants_from_second_question = await get_variants_by_question_id(service_client, sample_questions[1]["id"])
    variants_from_non_existing_question = await get_variants_by_question_id(service_client, "")

    assert len(variants_from_first_question) == 1
    assert variants_from_first_question == [first_variant]

    assert len(variants_from_second_question) == 1
    assert variants_from_second_question == [second_variant]

    assert len(variants_from_non_existing_question) == 0
