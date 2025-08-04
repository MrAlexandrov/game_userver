import uuid
import handlers.cruds_pb2 as service
# import handlers.cruds_pb2_grpc as create_pack_grpc


async def test_create_pack_grpc(grpc_handlers):
    title = 'New Title'
    request = service.CreatePackRequest(title=title) # type: ignore
    response = await grpc_handlers.CreatePack(request)
    assert response.pack.title == title


async def test_get_pack_by_id_grpc(grpc_handlers):
    # Сначала создаём пак
    title = "History Pack"
    create_request = service.CreatePackRequest(title=title) # type: ignore
    create_response = await grpc_handlers.CreatePack(create_request)
    pack_id = create_response.pack.id

    # Теперь получаем по ID
    get_request = service.GetPackByIdRequest(id=pack_id) # type: ignore
    get_response = await grpc_handlers.GetPackById(get_request)

    assert get_response.pack.id == pack_id
    assert get_response.pack.title == title


async def test_get_pack_by_id_not_found_grpc(grpc_handlers):
    # Несуществующий UUID
    fake_id = str(uuid.uuid4())
    request = service.GetPackByIdRequest(id=fake_id) # type: ignore
    try:
        await grpc_handlers.GetPackById(request)
        assert False, "Should have raised an error"
    except Exception as e:
        # В Userver gRPC при ошибке может прийти StatusCode.INTERNAL или NOT_FOUND
        # В зависимости от реализации throw
        assert "No such pack" in str(e) or "NotFound" in str(e) or "failed" in str(e).lower()


async def test_get_all_packs_grpc(grpc_handlers):
    # Создаём несколько паков
    titles = ["Math", "Physics", "Chemistry"]
    created_ids = []

    for title in titles:
        req = service.CreatePackRequest(title=title) # type: ignore
        resp = await grpc_handlers.CreatePack(req)
        created_ids.append(resp.pack.id)

    # Запрашиваем все
    request = service.GetAllPacksRequest()
    response = await grpc_handlers.GetAllPacks(request)

    # Проверяем, что все созданные паки есть в ответе
    response_titles = [pack.title for pack in response.packs]
    response_ids = [pack.id for pack in response.packs]

    assert len(response.packs) >= len(titles)
    for title in titles:
        assert title in response_titles
    for pack_id in created_ids:
        assert pack_id in response_ids


# === Тесты для Question ===

async def test_create_question_grpc(grpc_handlers):
    # Сначала создаём пак
    create_pack_req = service.CreatePackRequest(title="Geography") # type: ignore
    pack_resp = await grpc_handlers.CreatePack(create_pack_req)
    pack_id = pack_resp.pack.id

    # Создаём вопрос
    question_text = "What is the capital of France?"
    image_url = "http://example.com/paris.jpg"

    create_question_req = service.CreateQuestionRequest(
        pack_id=pack_id, # type: ignore
        text=question_text, # type: ignore
        image_url=image_url # type: ignore
    )
    question_resp = await grpc_handlers.CreateQuestion(create_question_req)

    assert question_resp.question.text == question_text
    assert question_resp.question.pack_id == pack_id
    assert question_resp.question.image_url == image_url
    assert uuid.UUID(question_resp.question.id)


async def test_get_question_by_id_grpc(grpc_handlers):
    # Создаём пак и вопрос
    pack_resp = await grpc_handlers.CreatePack(service.CreatePackRequest(title="Biology")) # type: ignore
    question_resp = await grpc_handlers.CreateQuestion(
        service.CreateQuestionRequest(
            pack_id=pack_resp.pack.id, # type: ignore
            text="What is DNA?", # type: ignore
            image_url="" # type: ignore
        )
    )
    question_id = question_resp.question.id

    # Получаем по ID
    get_req = service.GetQuestionByIdRequest(id=question_id) # type: ignore
    get_resp = await grpc_handlers.GetQuestionById(get_req)

    assert get_resp.question.id == question_id
    assert get_resp.question.text == "What is DNA?"
    assert get_resp.question.pack_id == pack_resp.pack.id


async def test_get_questions_by_pack_id_grpc(grpc_handlers):
    # Создаём пак
    pack_resp = await grpc_handlers.CreatePack(service.CreatePackRequest(title="Astronomy")) # type: ignore
    pack_id = pack_resp.pack.id

    # Добавляем несколько вопросов
    questions_data = [
        ("How many planets?", "http://example.com/solar.jpg"),
        ("What is a black hole?", ""),
        ("Star life cycle?", "http://example.com/star.png"),
    ]

    for text, img in questions_data:
        await grpc_handlers.CreateQuestion(
            service.CreateQuestionRequest(pack_id=pack_id, text=text, image_url=img) # type: ignore
        )

    # Получаем все вопросы пака
    req = service.GetQuestionsByPackIdRequest(pack_id=pack_id) # type: ignore
    resp = await grpc_handlers.GetQuestionsByPackId(req)

    assert len(resp.questions) == len(questions_data)
    response_texts = [q.text for q in resp.questions]
    for text, _ in questions_data:
        assert text in response_texts


# === Тесты для Variant ===

async def test_create_variant_grpc(grpc_handlers):
    # Создаём пак → вопрос → вариант
    pack_resp = await grpc_handlers.CreatePack(service.CreatePackRequest(title="Logic")) # type: ignore
    question_resp = await grpc_handlers.CreateQuestion(
        service.CreateQuestionRequest(
            pack_id=pack_resp.pack.id, # type: ignore
            text="2 + 2 = ?" # type: ignore
        )
    )
    question_id = question_resp.question.id

    # Создаём вариант
    variant_text = "4"
    is_correct = True

    variant_req = service.CreateVariantRequest(
        question_id=question_id, # type: ignore
        text=variant_text, # type: ignore
        is_correct=is_correct # type: ignore
    )
    variant_resp = await grpc_handlers.CreateVariant(variant_req)

    assert variant_resp.variant.text == variant_text
    assert variant_resp.variant.is_correct == is_correct
    assert variant_resp.variant.question_id == question_id
    assert uuid.UUID(variant_resp.variant.id)


async def test_get_variant_by_id_grpc(grpc_handlers):
    # Создаём пак → вопрос → вариант
    pack_resp = await grpc_handlers.CreatePack(service.CreatePackRequest(title="Music")) # type: ignore
    question_resp = await grpc_handlers.CreateQuestion(
        service.CreateQuestionRequest(pack_id=pack_resp.pack.id, text="Who wrote Symphony No. 9?") # type: ignore
    )
    variant_resp = await grpc_handlers.CreateVariant(
        service.CreateVariantRequest(
            question_id=question_resp.question.id, # type: ignore
            text="Beethoven", # type: ignore
            is_correct=True # type: ignore
        )
    )
    variant_id = variant_resp.variant.id

    # Получаем по ID
    get_req = service.GetVariantByIdRequest(id=variant_id) # type: ignore
    get_resp = await grpc_handlers.GetVariantById(get_req)

    assert get_resp.variant.id == variant_id
    assert get_resp.variant.text == "Beethoven"
    assert get_resp.variant.is_correct is True


async def test_get_variants_by_question_id_grpc(grpc_handlers):
    # Создаём пак → вопрос → несколько вариантов
    pack_resp = await grpc_handlers.CreatePack(service.CreatePackRequest(title="Sports")) # type: ignore
    question_resp = await grpc_handlers.CreateQuestion(
        service.CreateQuestionRequest(pack_id=pack_resp.pack.id, text="Which team won 2022 World Cup?") # type: ignore
    )
    question_id = question_resp.question.id

    variants_data = [
        ("Brazil", False),
        ("France", False),
        ("Argentina", True),
        ("Germany", False),
    ]

    for text, correct in variants_data:
        await grpc_handlers.CreateVariant(
            service.CreateVariantRequest(question_id=question_id, text=text, is_correct=correct) # type: ignore
        )

    # Получаем все варианты
    req = service.GetVariantsByQuestionIdRequest(question_id=question_id) # type: ignore
    resp = await grpc_handlers.GetVariantsByQuestionId(req)

    assert len(resp.variants) == len(variants_data)
    variant_map = {v.text: v.is_correct for v in resp.variants}
    for text, correct in variants_data:
        assert text in variant_map
        assert variant_map[text] == correct
