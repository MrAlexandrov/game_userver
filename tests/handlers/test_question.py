import pytest
from endpoints import (
    create_pack,

    create_question,
    get_question_by_id,
    get_questions_by_pack_id
)


@pytest.fixture
async def sample_packs(service_client):
    packs = []
    for i in range(2):
        pack = await create_pack(service_client, f'Pack {i}')
        packs.append(pack)
    return packs


async def test_create_question(service_client, sample_packs):
    await create_question(service_client, sample_packs[0]["id"], "some question?")


async def test_get_question_by_id(service_client, sample_packs):
    created_question = await create_question(service_client, sample_packs[0]["id"], "Question", "some url")
    getted_question = await get_question_by_id(service_client, created_question["id"])

    assert created_question == getted_question


async def test_get_questions_by_pack_id(service_client, sample_packs):
    first_question = await create_question(service_client, sample_packs[0]["id"], "first_question")
    second_question = await create_question(service_client, sample_packs[1]["id"], "second_question")

    questions_from_first_pack = await get_questions_by_pack_id(service_client, sample_packs[0]["id"])
    questions_from_second_pack = await get_questions_by_pack_id(service_client, sample_packs[1]["id"])
    questions_from_non_existing_pack = await get_questions_by_pack_id(service_client, "")

    assert len(questions_from_first_pack) == 1
    assert questions_from_first_pack == [first_question]

    assert len(questions_from_second_pack) == 1
    assert questions_from_second_pack == [second_question]

    assert len(questions_from_non_existing_pack) == 0
