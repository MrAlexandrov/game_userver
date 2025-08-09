import uuid

import pytest
import handlers.cruds_pb2 as service


# === Game Session Tests ===

async def test_create_game_session_grpc(grpc_handlers, created_pack_id):
    request = service.CreateGameSessionRequest(pack_id=created_pack_id)  # type: ignore
    response = await grpc_handlers.CreateGameSession(request)

    assert response.game_session.pack_id == created_pack_id
    assert response.game_session.state == "waiting"
    assert response.game_session.current_question_index == 0
    assert uuid.UUID(response.game_session.id), "ID should be valid UUID"
    assert response.game_session.created_at > 0
    assert not response.game_session.started_at
    assert not response.game_session.finished_at


async def test_get_game_session_grpc(grpc_handlers, created_pack_id):
    # First create a game session
    create_request = service.CreateGameSessionRequest(pack_id=created_pack_id)  # type: ignore
    create_response = await grpc_handlers.CreateGameSession(create_request)
    game_session_id = create_response.game_session.id

    # Then retrieve it
    get_request = service.GetGameSessionRequest(id=game_session_id)  # type: ignore
    get_response = await grpc_handlers.GetGameSession(get_request)

    assert get_response.game_session.id == game_session_id
    assert get_response.game_session.pack_id == created_pack_id
    assert get_response.game_session.state == "waiting"
    assert get_response.game_session.current_question_index == 0


async def test_get_game_session_not_found_grpc(grpc_handlers):
    fake_id = str(uuid.uuid4())
    request = service.GetGameSessionRequest(id=fake_id)  # type: ignore

    with pytest.raises(Exception) as exc_info:
        await grpc_handlers.GetGameSession(request)

    # Проверяем, что это gRPC-ошибка с NOT_FOUND
    assert "NOT_FOUND" in str(exc_info.value) or "Game session not found" in str(exc_info.value)


async def test_start_game_session_grpc(grpc_handlers, created_pack_id):
    # First create a game session
    create_request = service.CreateGameSessionRequest(pack_id=created_pack_id)  # type: ignore
    create_response = await grpc_handlers.CreateGameSession(create_request)
    game_session_id = create_response.game_session.id

    # Start the game session
    start_request = service.StartGameSessionRequest(id=game_session_id)  # type: ignore
    start_response = await grpc_handlers.StartGameSession(start_request)

    assert start_response.game_session.id == game_session_id
    assert start_response.game_session.state == "active"
    assert start_response.game_session.started_at > 0
    assert not start_response.game_session.finished_at


async def test_end_game_session_grpc(grpc_handlers, created_pack_id):
    # First create a game session
    create_request = service.CreateGameSessionRequest(pack_id=created_pack_id)  # type: ignore
    create_response = await grpc_handlers.CreateGameSession(create_request)
    game_session_id = create_response.game_session.id

    # Start the game session
    start_request = service.StartGameSessionRequest(id=game_session_id)  # type: ignore
    await grpc_handlers.StartGameSession(start_request)

    # End the game session
    end_request = service.EndGameSessionRequest(id=game_session_id)  # type: ignore
    end_response = await grpc_handlers.EndGameSession(end_request)

    assert end_response.game_session.id == game_session_id
    assert end_response.game_session.state == "finished"
    assert end_response.game_session.finished_at > 0


async def test_get_all_game_sessions_grpc(grpc_handlers, created_pack_id):
    # Create a game session
    create_request = service.CreateGameSessionRequest(pack_id=created_pack_id)  # type: ignore
    create_response = await grpc_handlers.CreateGameSession(create_request)
    game_session_id = create_response.game_session.id

    # Get all game sessions
    request = service.GetGameSessionsRequest()
    response = await grpc_handlers.GetGameSessions(request)

    game_session_ids = [gs.id for gs in response.game_sessions]
    assert game_session_id in game_session_ids
    assert len(response.game_sessions) >= 1


# === Player Tests ===

async def test_add_player_grpc(grpc_handlers, created_pack_id):
    # First create a game session
    create_request = service.CreateGameSessionRequest(pack_id=created_pack_id)  # type: ignore
    create_response = await grpc_handlers.CreateGameSession(create_request)
    game_session_id = create_response.game_session.id

    # Add a player
    player_name = "Alice"
    add_request = service.AddPlayerRequest(
        game_session_id=game_session_id,  # type: ignore
        name=player_name  # type: ignore
    )
    add_response = await grpc_handlers.AddPlayer(add_request)

    assert add_response.player.name == player_name
    assert add_response.player.game_session_id == game_session_id
    assert add_response.player.score == 0
    assert uuid.UUID(add_response.player.id), "ID should be valid UUID"
    assert add_response.player.joined_at > 0


async def test_get_players_grpc(grpc_handlers, created_pack_id):
    # First create a game session
    create_request = service.CreateGameSessionRequest(pack_id=created_pack_id)  # type: ignore
    create_response = await grpc_handlers.CreateGameSession(create_request)
    game_session_id = create_response.game_session.id

    # Add players
    player_names = ["Alice", "Bob"]
    player_ids = []
    for name in player_names:
        add_request = service.AddPlayerRequest(
            game_session_id=game_session_id,  # type: ignore
            name=name  # type: ignore
        )
        add_response = await grpc_handlers.AddPlayer(add_request)
        player_ids.append(add_response.player.id)

    # Get players
    get_request = service.GetPlayersRequest(game_session_id=game_session_id)  # type: ignore
    get_response = await grpc_handlers.GetPlayers(get_request)

    assert len(get_response.players) == 2
    player_names_in_response = [p.name for p in get_response.players]
    player_ids_in_response = [p.id for p in get_response.players]
    
    assert set(player_names) == set(player_names_in_response)
    assert set(player_ids) == set(player_ids_in_response)


# === Player Answer Tests ===

async def test_submit_answer_grpc(
    grpc_handlers, 
    created_pack_id, 
    created_question_id, 
    created_variants_ids
):
    # First create a game session
    create_request = service.CreateGameSessionRequest(pack_id=created_pack_id)  # type: ignore
    create_response = await grpc_handlers.CreateGameSession(create_request)
    game_session_id = create_response.game_session.id

    # Add a player
    player_name = "Alice"
    add_request = service.AddPlayerRequest(
        game_session_id=game_session_id,  # type: ignore
        name=player_name  # type: ignore
    )
    add_response = await grpc_handlers.AddPlayer(add_request)
    player_id = add_response.player.id

    # Find the correct variant
    get_variants_request = service.GetVariantsByQuestionIdRequest(question_id=created_question_id)  # type: ignore
    get_variants_response = await grpc_handlers.GetVariantsByQuestionId(get_variants_request)
    
    correct_variant = None
    for variant in get_variants_response.variants:
        if variant.is_correct:
            correct_variant = variant
            break
    
    assert correct_variant is not None, "Should have a correct variant"

    # Submit answer
    submit_request = service.SubmitAnswerRequest(
        player_id=player_id,  # type: ignore
        question_id=created_question_id,  # type: ignore
        variant_id=correct_variant.id  # type: ignore
    )
    submit_response = await grpc_handlers.SubmitAnswer(submit_request)

    assert submit_response.is_correct == True
    assert submit_response.points == 10


async def test_get_player_answers_grpc(
    grpc_handlers, 
    created_pack_id, 
    created_question_id, 
    created_variants_ids
):
    # First create a game session
    create_request = service.CreateGameSessionRequest(pack_id=created_pack_id)  # type: ignore
    create_response = await grpc_handlers.CreateGameSession(create_request)
    game_session_id = create_response.game_session.id

    # Add a player
    player_name = "Alice"
    add_request = service.AddPlayerRequest(
        game_session_id=game_session_id,  # type: ignore
        name=player_name  # type: ignore
    )
    add_response = await grpc_handlers.AddPlayer(add_request)
    player_id = add_response.player.id

    # Find the correct variant
    get_variants_request = service.GetVariantsByQuestionIdRequest(question_id=created_question_id)  # type: ignore
    get_variants_response = await grpc_handlers.GetVariantsByQuestionId(get_variants_request)
    
    correct_variant = None
    for variant in get_variants_response.variants:
        if variant.is_correct:
            correct_variant = variant
            break
    
    assert correct_variant is not None, "Should have a correct variant"

    # Submit answer
    submit_request = service.SubmitAnswerRequest(
        player_id=player_id,  # type: ignore
        question_id=created_question_id,  # type: ignore
        variant_id=correct_variant.id  # type: ignore
    )
    await grpc_handlers.SubmitAnswer(submit_request)

    # Get player answers
    get_answers_request = service.GetPlayerAnswersRequest(player_id=player_id)  # type: ignore
    get_answers_response = await grpc_handlers.GetPlayerAnswers(get_answers_request)

    assert len(get_answers_response.answers) == 1
    answer = get_answers_response.answers[0]
    assert answer.player_id == player_id
    assert answer.question_id == created_question_id
    assert answer.variant_id == correct_variant.id
    assert answer.is_correct == True
    assert answer.answered_at > 0