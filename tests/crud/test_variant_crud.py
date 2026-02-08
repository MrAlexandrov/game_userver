"""
Integration tests for Variant CRUD operations.
Tests verify that variant operations work correctly with the database.
"""

import pytest
import handlers.cruds_pb2 as service


@pytest.mark.pgsql('db_1')
async def test_create_variant(grpc_handlers, created_question_id, pgsql):
    """Test creating a variant and verifying it exists in the database"""
    variant_text = "Paris"
    is_correct = True
    
    # Create variant via gRPC
    request = service.CreateVariantRequest(
        question_id=created_question_id,  # type: ignore
        text=variant_text,  # type: ignore
        is_correct=is_correct  # type: ignore
    )
    response = await grpc_handlers.CreateVariant(request)
    
    # Verify response
    assert response.variant.id is not None
    assert response.variant.question_id == created_question_id
    assert response.variant.text == variant_text
    assert response.variant.is_correct == is_correct
    
    # Verify in database
    cursor = pgsql['db_1'].cursor()
    cursor.execute(
        "SELECT id, question_id, text, is_correct FROM quiz.variants WHERE id = %s",
        (response.variant.id,)
    )
    row = cursor.fetchone()
    assert row is not None
    assert row[1] == created_question_id
    assert row[2] == variant_text
    assert row[3] == is_correct


@pytest.mark.pgsql('db_1')
async def test_get_variant_by_id(grpc_handlers, created_question_id):
    """Test retrieving a variant by ID"""
    variant_text = "London"
    is_correct = False
    
    # Create variant
    create_request = service.CreateVariantRequest(
        question_id=created_question_id,  # type: ignore
        text=variant_text,  # type: ignore
        is_correct=is_correct  # type: ignore
    )
    create_response = await grpc_handlers.CreateVariant(create_request)
    variant_id = create_response.variant.id
    
    # Get variant by ID
    get_request = service.GetVariantByIdRequest(id=variant_id)  # type: ignore
    get_response = await grpc_handlers.GetVariantById(get_request)
    
    # Verify response
    assert get_response.variant.id == variant_id
    assert get_response.variant.text == variant_text
    assert get_response.variant.is_correct == is_correct


@pytest.mark.pgsql('db_1')
async def test_get_variants_by_question_id(grpc_handlers, created_question_id):
    """Test retrieving all variants for a specific question"""
    # Create multiple variants
    variants_data = [
        ("Paris", True),
        ("London", False),
        ("Berlin", False),
        ("Madrid", False)
    ]
    created_ids = []
    
    for text, is_correct in variants_data:
        request = service.CreateVariantRequest(
            question_id=created_question_id,  # type: ignore
            text=text,  # type: ignore
            is_correct=is_correct  # type: ignore
        )
        response = await grpc_handlers.CreateVariant(request)
        created_ids.append(response.variant.id)
    
    # Get all variants for the question
    get_request = service.GetVariantsByQuestionIdRequest(question_id=created_question_id)  # type: ignore
    get_response = await grpc_handlers.GetVariantsByQuestionId(get_request)
    
    # Verify all created variants are in the response
    response_ids = [v.id for v in get_response.variants]
    for variant_id in created_ids:
        assert variant_id in response_ids
    
    # Verify correct count
    assert len(get_response.variants) == len(variants_data)


@pytest.mark.pgsql('db_1')
async def test_multiple_correct_variants(grpc_handlers, created_question_id):
    """Test creating multiple correct variants (for multi-select questions)"""
    # Create two correct variants
    variant1_request = service.CreateVariantRequest(
        question_id=created_question_id,  # type: ignore
        text="Correct Answer 1",  # type: ignore
        is_correct=True  # type: ignore
    )
    variant1_response = await grpc_handlers.CreateVariant(variant1_request)
    
    variant2_request = service.CreateVariantRequest(
        question_id=created_question_id,  # type: ignore
        text="Correct Answer 2",  # type: ignore
        is_correct=True  # type: ignore
    )
    variant2_response = await grpc_handlers.CreateVariant(variant2_request)
    
    # Verify both are marked as correct
    assert variant1_response.variant.is_correct is True
    assert variant2_response.variant.is_correct is True


@pytest.mark.pgsql('db_1')
async def test_variant_cascade_delete(grpc_handlers, pgsql):
    """Test that deleting a question cascades to delete its variants"""
    # Create a pack
    pack_request = service.CreatePackRequest(title="Pack for Cascade Test")  # type: ignore
    pack_response = await grpc_handlers.CreatePack(pack_request)
    pack_id = pack_response.pack.id
    
    # Create a question
    question_request = service.CreateQuestionRequest(
        pack_id=pack_id,  # type: ignore
        text="Question for cascade test",  # type: ignore
        image_url=""  # type: ignore
    )
    question_response = await grpc_handlers.CreateQuestion(question_request)
    question_id = question_response.question.id
    
    # Create a variant
    variant_request = service.CreateVariantRequest(
        question_id=question_id,  # type: ignore
        text="Variant to be deleted",  # type: ignore
        is_correct=True  # type: ignore
    )
    variant_response = await grpc_handlers.CreateVariant(variant_request)
    variant_id = variant_response.variant.id
    
    # Delete the question directly in database
    cursor = pgsql['db_1'].cursor()
    cursor.execute("DELETE FROM quiz.questions WHERE id = %s", (question_id,))
    pgsql['db_1'].commit()
    
    # Verify variant was also deleted (cascade)
    cursor.execute("SELECT COUNT(*) FROM quiz.variants WHERE id = %s", (variant_id,))
    count = cursor.fetchone()[0]
    assert count == 0


@pytest.mark.pgsql('db_1')
async def test_variant_correctness_flag(grpc_handlers, created_question_id, pgsql):
    """Test that is_correct flag is properly stored and retrieved"""
    # Create correct variant
    correct_request = service.CreateVariantRequest(
        question_id=created_question_id,  # type: ignore
        text="Correct",  # type: ignore
        is_correct=True  # type: ignore
    )
    correct_response = await grpc_handlers.CreateVariant(correct_request)
    
    # Create incorrect variant
    incorrect_request = service.CreateVariantRequest(
        question_id=created_question_id,  # type: ignore
        text="Incorrect",  # type: ignore
        is_correct=False  # type: ignore
    )
    incorrect_response = await grpc_handlers.CreateVariant(incorrect_request)
    
    # Verify in database
    cursor = pgsql['db_1'].cursor()
    cursor.execute(
        "SELECT is_correct FROM quiz.variants WHERE id = %s",
        (correct_response.variant.id,)
    )
    assert cursor.fetchone()[0] is True
    
    cursor.execute(
        "SELECT is_correct FROM quiz.variants WHERE id = %s",
        (incorrect_response.variant.id,)
    )
    assert cursor.fetchone()[0] is False