"""
End-to-end integration tests for complete CRUD scenarios.
Tests verify that the entire workflow from pack creation to variants works correctly.
"""

import pytest
import handlers.cruds_pb2 as service


@pytest.mark.pgsql('db_1')
async def test_complete_pack_creation_scenario(grpc_handlers, pgsql):
    """
    Test complete scenario: Create pack → Create questions → Create variants
    Verifies the entire workflow works and data is properly stored in database
    """
    # Step 1: Create a pack
    pack_title = "Geography Quiz"
    pack_request = service.CreatePackRequest(title=pack_title)  # type: ignore
    pack_response = await grpc_handlers.CreatePack(pack_request)
    pack_id = pack_response.pack.id
    
    assert pack_id is not None
    assert pack_response.pack.title == pack_title
    
    # Verify pack in database
    cursor = pgsql['db_1'].cursor()
    cursor.execute("SELECT title FROM quiz.packs WHERE id = %s", (pack_id,))
    assert cursor.fetchone()[0] == pack_title
    
    # Step 2: Create multiple questions in the pack
    questions_data = [
        ("What is the capital of France?", "http://example.com/france.jpg"),
        ("What is the capital of Germany?", "http://example.com/germany.jpg"),
        ("What is the capital of Italy?", "")
    ]
    question_ids = []
    
    for question_text, image_url in questions_data:
        question_request = service.CreateQuestionRequest(
            pack_id=pack_id,  # type: ignore
            text=question_text,  # type: ignore
            image_url=image_url  # type: ignore
        )
        question_response = await grpc_handlers.CreateQuestion(question_request)
        question_id = question_response.question.id
        question_ids.append(question_id)
        
        assert question_id is not None
        assert question_response.question.pack_id == pack_id
        assert question_response.question.text == question_text
    
    # Verify questions in database
    cursor.execute(
        "SELECT COUNT(*) FROM quiz.questions WHERE pack_id = %s",
        (pack_id,)
    )
    assert cursor.fetchone()[0] == len(questions_data)
    
    # Step 3: Create variants for each question
    variants_per_question = [
        [("Paris", True), ("London", False), ("Berlin", False), ("Madrid", False)],
        [("Berlin", True), ("Paris", False), ("Rome", False), ("Vienna", False)],
        [("Rome", True), ("Milan", False), ("Venice", False), ("Florence", False)]
    ]
    
    for question_id, variants_data in zip(question_ids, variants_per_question):
        variant_ids = []
        for variant_text, is_correct in variants_data:
            variant_request = service.CreateVariantRequest(
                question_id=question_id,  # type: ignore
                text=variant_text,  # type: ignore
                is_correct=is_correct  # type: ignore
            )
            variant_response = await grpc_handlers.CreateVariant(variant_request)
            variant_id = variant_response.variant.id
            variant_ids.append(variant_id)
            
            assert variant_id is not None
            assert variant_response.variant.question_id == question_id
            assert variant_response.variant.text == variant_text
            assert variant_response.variant.is_correct == is_correct
        
        # Verify variants in database for this question
        cursor.execute(
            "SELECT COUNT(*) FROM quiz.variants WHERE question_id = %s",
            (question_id,)
        )
        assert cursor.fetchone()[0] == len(variants_data)
    
    # Step 4: Verify complete structure via gRPC
    # Get all questions for the pack
    get_questions_request = service.GetQuestionsByPackIdRequest(pack_id=pack_id)  # type: ignore
    get_questions_response = await grpc_handlers.GetQuestionsByPackId(get_questions_request)
    assert len(get_questions_response.questions) == len(questions_data)
    
    # Get variants for each question
    for question_id in question_ids:
        get_variants_request = service.GetVariantsByQuestionIdRequest(question_id=question_id)  # type: ignore
        get_variants_response = await grpc_handlers.GetVariantsByQuestionId(get_variants_request)
        assert len(get_variants_response.variants) == 4  # Each question has 4 variants


@pytest.mark.pgsql('db_1')
async def test_pack_with_single_question_and_variants(grpc_handlers, pgsql):
    """Test creating a minimal complete pack with one question and its variants"""
    # Create pack
    pack_request = service.CreatePackRequest(title="Simple Quiz")  # type: ignore
    pack_response = await grpc_handlers.CreatePack(pack_request)
    pack_id = pack_response.pack.id
    
    # Create question
    question_request = service.CreateQuestionRequest(
        pack_id=pack_id,  # type: ignore
        text="Is the sky blue?",  # type: ignore
        image_url=""  # type: ignore
    )
    question_response = await grpc_handlers.CreateQuestion(question_request)
    question_id = question_response.question.id
    
    # Create variants
    variants = [("Yes", True), ("No", False)]
    for text, is_correct in variants:
        variant_request = service.CreateVariantRequest(
            question_id=question_id,  # type: ignore
            text=text,  # type: ignore
            is_correct=is_correct  # type: ignore
        )
        await grpc_handlers.CreateVariant(variant_request)
    
    # Verify complete structure in database
    cursor = pgsql['db_1'].cursor()
    
    # Check pack exists
    cursor.execute("SELECT COUNT(*) FROM quiz.packs WHERE id = %s", (pack_id,))
    assert cursor.fetchone()[0] == 1
    
    # Check question exists
    cursor.execute("SELECT COUNT(*) FROM quiz.questions WHERE id = %s", (question_id,))
    assert cursor.fetchone()[0] == 1
    
    # Check variants exist
    cursor.execute("SELECT COUNT(*) FROM quiz.variants WHERE question_id = %s", (question_id,))
    assert cursor.fetchone()[0] == 2


@pytest.mark.pgsql('db_1')
async def test_retrieve_complete_pack_structure(grpc_handlers):
    """Test retrieving a complete pack with all its questions and variants"""
    # Create pack
    pack_request = service.CreatePackRequest(title="Complete Pack")  # type: ignore
    pack_response = await grpc_handlers.CreatePack(pack_request)
    pack_id = pack_response.pack.id
    
    # Create 2 questions
    question_ids = []
    for i in range(2):
        question_request = service.CreateQuestionRequest(
            pack_id=pack_id,  # type: ignore
            text=f"Question {i+1}",  # type: ignore
            image_url=""  # type: ignore
        )
        question_response = await grpc_handlers.CreateQuestion(question_request)
        question_ids.append(question_response.question.id)
    
    # Create 3 variants for each question
    for question_id in question_ids:
        for j in range(3):
            variant_request = service.CreateVariantRequest(
                question_id=question_id,  # type: ignore
                text=f"Variant {j+1}",  # type: ignore
                is_correct=(j == 0)  # type: ignore  # First variant is correct
            )
            await grpc_handlers.CreateVariant(variant_request)
    
    # Now retrieve everything
    # 1. Get pack
    get_pack_request = service.GetPackByIdRequest(id=pack_id)  # type: ignore
    get_pack_response = await grpc_handlers.GetPackById(get_pack_request)
    assert get_pack_response.pack.id == pack_id
    
    # 2. Get all questions
    get_questions_request = service.GetQuestionsByPackIdRequest(pack_id=pack_id)  # type: ignore
    get_questions_response = await grpc_handlers.GetQuestionsByPackId(get_questions_request)
    assert len(get_questions_response.questions) == 2
    
    # 3. Get variants for each question
    for question in get_questions_response.questions:
        get_variants_request = service.GetVariantsByQuestionIdRequest(question_id=question.id)  # type: ignore
        get_variants_response = await grpc_handlers.GetVariantsByQuestionId(get_variants_request)
        assert len(get_variants_response.variants) == 3
        
        # Verify exactly one correct variant
        correct_count = sum(1 for v in get_variants_response.variants if v.is_correct)
        assert correct_count == 1


@pytest.mark.pgsql('db_1')
async def test_cascade_delete_complete_structure(grpc_handlers, pgsql):
    """Test that deleting a pack cascades to delete all questions and variants"""
    # Create complete structure
    pack_request = service.CreatePackRequest(title="Pack to Delete")  # type: ignore
    pack_response = await grpc_handlers.CreatePack(pack_request)
    pack_id = pack_response.pack.id
    
    # Create question
    question_request = service.CreateQuestionRequest(
        pack_id=pack_id,  # type: ignore
        text="Question to delete",  # type: ignore
        image_url=""  # type: ignore
    )
    question_response = await grpc_handlers.CreateQuestion(question_request)
    question_id = question_response.question.id
    
    # Create variant
    variant_request = service.CreateVariantRequest(
        question_id=question_id,  # type: ignore
        text="Variant to delete",  # type: ignore
        is_correct=True  # type: ignore
    )
    variant_response = await grpc_handlers.CreateVariant(variant_request)
    variant_id = variant_response.variant.id
    
    # Delete pack
    cursor = pgsql['db_1'].cursor()
    cursor.execute("DELETE FROM quiz.packs WHERE id = %s", (pack_id,))
    pgsql['db_1'].commit()
    
    # Verify everything was deleted
    cursor.execute("SELECT COUNT(*) FROM quiz.packs WHERE id = %s", (pack_id,))
    assert cursor.fetchone()[0] == 0
    
    cursor.execute("SELECT COUNT(*) FROM quiz.questions WHERE id = %s", (question_id,))
    assert cursor.fetchone()[0] == 0
    
    cursor.execute("SELECT COUNT(*) FROM quiz.variants WHERE id = %s", (variant_id,))
    assert cursor.fetchone()[0] == 0


@pytest.mark.pgsql('db_1')
async def test_multiple_packs_isolation(grpc_handlers):
    """Test that multiple packs are properly isolated from each other"""
    # Create two separate packs
    pack1_request = service.CreatePackRequest(title="Pack 1")  # type: ignore
    pack1_response = await grpc_handlers.CreatePack(pack1_request)
    pack1_id = pack1_response.pack.id
    
    pack2_request = service.CreatePackRequest(title="Pack 2")  # type: ignore
    pack2_response = await grpc_handlers.CreatePack(pack2_request)
    pack2_id = pack2_response.pack.id
    
    # Create questions in each pack
    question1_request = service.CreateQuestionRequest(
        pack_id=pack1_id,  # type: ignore
        text="Question in Pack 1",  # type: ignore
        image_url=""  # type: ignore
    )
    question1_response = await grpc_handlers.CreateQuestion(question1_request)
    
    question2_request = service.CreateQuestionRequest(
        pack_id=pack2_id,  # type: ignore
        text="Question in Pack 2",  # type: ignore
        image_url=""  # type: ignore
    )
    question2_response = await grpc_handlers.CreateQuestion(question2_request)
    
    # Verify questions are in correct packs
    get_pack1_questions = service.GetQuestionsByPackIdRequest(pack_id=pack1_id)  # type: ignore
    pack1_questions_response = await grpc_handlers.GetQuestionsByPackId(get_pack1_questions)
    assert len(pack1_questions_response.questions) == 1
    assert pack1_questions_response.questions[0].id == question1_response.question.id
    
    get_pack2_questions = service.GetQuestionsByPackIdRequest(pack_id=pack2_id)  # type: ignore
    pack2_questions_response = await grpc_handlers.GetQuestionsByPackId(get_pack2_questions)
    assert len(pack2_questions_response.questions) == 1
    assert pack2_questions_response.questions[0].id == question2_response.question.id