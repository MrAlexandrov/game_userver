import uuid

import pytest
import handlers.cruds_pb2 as service
# import handlers.cruds_pb2_grpc as create_pack_grpc


async def test_create_pack_grpc(grpc_handlers, sample_pack_title):
    request = service.CreatePackRequest(title=sample_pack_title) # type: ignore
    response = await grpc_handlers.CreatePack(request)

    assert response.pack.title == sample_pack_title
    assert uuid.UUID(response.pack.id), "ID should be valid UUID"


async def test_get_pack_by_id_grpc(grpc_handlers, created_pack_id, sample_pack_title):
    request = service.GetPackByIdRequest(id=created_pack_id) # type: ignore
    response = await grpc_handlers.GetPackById(request)

    assert response.pack.id == created_pack_id
    assert response.pack.title == sample_pack_title


async def test_get_pack_by_id_not_found_grpc(grpc_handlers):
    fake_id = str(uuid.uuid4())
    request = service.GetPackByIdRequest(id=fake_id) # type: ignore

    with pytest.raises(Exception) as exc_info:
        await grpc_handlers.GetPackById(request)

    # Проверяем, что это gRPC-ошибка с NOT_FOUND
    assert "NOT_FOUND" in str(exc_info.value) or "No such pack" in str(exc_info.value)


async def test_get_all_packs_grpc(grpc_handlers, created_pack_id, sample_pack_title):
    request = service.GetAllPacksRequest()
    response = await grpc_handlers.GetAllPacks(request)

    titles = [p.title for p in response.packs]
    ids = [p.id for p in response.packs]

    assert sample_pack_title in titles
    assert created_pack_id in ids


# === Тесты для Question ===

async def test_create_question_grpc(
    grpc_handlers,
    created_pack_id,
    sample_question_text,
    sample_image_url
):
    request = service.CreateQuestionRequest(
        pack_id=created_pack_id, # type: ignore
        text=sample_question_text, # type: ignore
        image_url=sample_image_url # type: ignore
    )
    response = await grpc_handlers.CreateQuestion(request)

    assert response.question.text == sample_question_text
    assert response.question.pack_id == created_pack_id
    assert response.question.image_url == sample_image_url
    assert uuid.UUID(response.question.id)


async def test_get_question_by_id_grpc(
    grpc_handlers,
    created_question_id,
    sample_question_text
):
    request = service.GetQuestionByIdRequest(id=created_question_id) # type: ignore
    response = await grpc_handlers.GetQuestionById(request)

    assert response.question.id == created_question_id
    assert response.question.text == sample_question_text


async def test_get_questions_by_pack_id_grpc(
    grpc_handlers,
    created_pack_id,
    created_question_id
):
    request = service.GetQuestionsByPackIdRequest(pack_id=created_pack_id) # type: ignore
    response = await grpc_handlers.GetQuestionsByPackId(request)

    question_ids = [q.id for q in response.questions]
    assert created_question_id in question_ids
    assert len(response.questions) == 1


# === Тесты для Variant ===

async def test_create_variant_grpc(
    grpc_handlers,
    created_question_id,
    sample_variant_data
):
    text, is_correct = sample_variant_data[0]  # например, "London", False

    request = service.CreateVariantRequest(
        question_id=created_question_id, # type: ignore
        text=text, # type: ignore
        is_correct=is_correct # type: ignore
    )
    response = await grpc_handlers.CreateVariant(request)

    assert response.variant.text == text
    assert response.variant.is_correct == is_correct
    assert response.variant.question_id == created_question_id
    assert uuid.UUID(response.variant.id)


async def test_get_variant_by_id_grpc(
    grpc_handlers,
    created_variants_ids,
    sample_variant_data
):
    # Берём ID первого варианта (например, "Beethoven")
    variant_id = created_variants_ids[0]
    expected_text, expected_correct = sample_variant_data[0]

    # Делаем запрос
    request = service.GetVariantByIdRequest(id=variant_id) # type: ignore
    response = await grpc_handlers.GetVariantById(request)

    # Проверяем
    assert response.variant.id == variant_id
    assert response.variant.text == expected_text
    assert response.variant.is_correct == expected_correct


async def test_get_variants_by_question_id_grpc(
    grpc_handlers,
    created_question_id,
    created_variants_ids
):
    request = service.GetVariantsByQuestionIdRequest(question_id=created_question_id) # type: ignore
    response = await grpc_handlers.GetVariantsByQuestionId(request)

    variant_ids = [v.id for v in response.variants]
    assert len(variant_ids) == len(created_variants_ids)
    assert set(variant_ids) == set(created_variants_ids)

    correct_found = False
    for variant in response.variants:
        if variant.is_correct:
            correct_found = True
            assert variant.text == "Paris"
    assert correct_found


# === Game Session Tests ===
# These are tested in a separate file test_game_session.py
