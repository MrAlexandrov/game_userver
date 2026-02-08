"""
Integration tests for Question CRUD operations.
Tests verify that question operations work correctly with the database.
"""

import pytest
import handlers.cruds_pb2 as service


@pytest.mark.pgsql('db_1')
async def test_create_question(grpc_handlers, created_pack_id, pgsql):
    """Test creating a question and verifying it exists in the database"""
    question_text = "What is 2+2?"
    image_url = "http://example.com/math.jpg"
    
    # Create question via gRPC
    request = service.CreateQuestionRequest(
        pack_id=created_pack_id,  # type: ignore
        text=question_text,  # type: ignore
        image_url=image_url  # type: ignore
    )
    response = await grpc_handlers.CreateQuestion(request)
    
    # Verify response
    assert response.question.id is not None
    assert response.question.pack_id == created_pack_id
    assert response.question.text == question_text
    assert response.question.image_url == image_url
    
    # Verify in database
    cursor = pgsql['db_1'].cursor()
    cursor.execute(
        "SELECT id, pack_id, text, image_url FROM quiz.questions WHERE id = %s",
        (response.question.id,)
    )
    row = cursor.fetchone()
    assert row is not None
    assert row[1] == created_pack_id
    assert row[2] == question_text
    assert row[3] == image_url


@pytest.mark.pgsql('db_1')
async def test_get_question_by_id(grpc_handlers, created_pack_id):
    """Test retrieving a question by ID"""
    question_text = "Question to Retrieve"
    
    # Create question
    create_request = service.CreateQuestionRequest(
        pack_id=created_pack_id,  # type: ignore
        text=question_text,  # type: ignore
        image_url="http://example.com/test.jpg"  # type: ignore
    )
    create_response = await grpc_handlers.CreateQuestion(create_request)
    question_id = create_response.question.id
    
    # Get question by ID
    get_request = service.GetQuestionByIdRequest(id=question_id)  # type: ignore
    get_response = await grpc_handlers.GetQuestionById(get_request)
    
    # Verify response
    assert get_response.question.id == question_id
    assert get_response.question.text == question_text
    assert get_response.question.pack_id == created_pack_id


@pytest.mark.pgsql('db_1')
async def test_get_questions_by_pack_id(grpc_handlers, created_pack_id):
    """Test retrieving all questions for a specific pack"""
    # Create multiple questions
    question_texts = ["Question 1", "Question 2", "Question 3"]
    created_ids = []
    
    for text in question_texts:
        request = service.CreateQuestionRequest(
            pack_id=created_pack_id,  # type: ignore
            text=text,  # type: ignore
            image_url=""  # type: ignore
        )
        response = await grpc_handlers.CreateQuestion(request)
        created_ids.append(response.question.id)
    
    # Get all questions for the pack
    get_request = service.GetQuestionsByPackIdRequest(pack_id=created_pack_id)  # type: ignore
    get_response = await grpc_handlers.GetQuestionsByPackId(get_request)
    
    # Verify all created questions are in the response
    response_ids = [q.id for q in get_response.questions]
    for question_id in created_ids:
        assert question_id in response_ids


@pytest.mark.pgsql('db_1')
async def test_question_without_image_url(grpc_handlers, created_pack_id):
    """Test creating a question without an image URL"""
    question_text = "Question without image"
    
    # Create question without image_url
    request = service.CreateQuestionRequest(
        pack_id=created_pack_id,  # type: ignore
        text=question_text,  # type: ignore
        image_url=""  # type: ignore
    )
    response = await grpc_handlers.CreateQuestion(request)
    
    # Verify response
    assert response.question.id is not None
    assert response.question.text == question_text
    assert response.question.image_url == ""


@pytest.mark.pgsql('db_1')
async def test_question_cascade_delete(grpc_handlers, pgsql):
    """Test that deleting a pack cascades to delete its questions"""
    # Create a pack
    pack_request = service.CreatePackRequest(title="Pack to Delete")  # type: ignore
    pack_response = await grpc_handlers.CreatePack(pack_request)
    pack_id = pack_response.pack.id
    
    # Create a question in that pack
    question_request = service.CreateQuestionRequest(
        pack_id=pack_id,  # type: ignore
        text="Question in pack to delete",  # type: ignore
        image_url=""  # type: ignore
    )
    question_response = await grpc_handlers.CreateQuestion(question_request)
    question_id = question_response.question.id
    
    # Delete the pack directly in database
    cursor = pgsql['db_1'].cursor()
    cursor.execute("DELETE FROM quiz.packs WHERE id = %s", (pack_id,))
    pgsql['db_1'].commit()
    
    # Verify question was also deleted (cascade)
    cursor.execute("SELECT COUNT(*) FROM quiz.questions WHERE id = %s", (question_id,))
    count = cursor.fetchone()[0]
    assert count == 0