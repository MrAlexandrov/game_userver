"""
Integration tests for the complete game lifecycle (happy path).

Tests verify the full flow: create game -> add players -> start -> play -> results.
Content (packs, questions, variants) is set up via gRPC fixtures.
Game flow is tested via HTTP REST API.
"""

import json

import pytest

from helpers.game_helpers import (
    create_game,
    add_player,
    start_game,
    get_game_state,
    submit_answer,
    get_game_results,
)


# =============================================================================
# Game session creation
# =============================================================================


async def test_create_game_session(service_client, quiz_pack):
    """Creating a game session returns correct initial state."""
    game = await create_game(service_client, quiz_pack["pack_id"])

    assert "id" in game
    assert game["pack_id"] == quiz_pack["pack_id"]
    assert game["state"] == "waiting"
    assert game["current_question_index"] == 0


# =============================================================================
# Adding players
# =============================================================================


async def test_add_player_to_game(service_client, quiz_pack):
    """Adding a player to a waiting game returns player info with score 0."""
    game = await create_game(service_client, quiz_pack["pack_id"])
    player = await add_player(service_client, game["id"], "Alice")

    assert "id" in player
    assert player["game_session_id"] == game["id"]
    assert player["name"] == "Alice"
    assert player["score"] == 0


async def test_add_multiple_players(service_client, quiz_pack):
    """Multiple players can join a waiting game."""
    game = await create_game(service_client, quiz_pack["pack_id"])

    p1 = await add_player(service_client, game["id"], "Alice")
    p2 = await add_player(service_client, game["id"], "Bob")
    p3 = await add_player(service_client, game["id"], "Charlie")

    assert p1["name"] == "Alice"
    assert p2["name"] == "Bob"
    assert p3["name"] == "Charlie"

    # All players have unique IDs
    ids = {p1["id"], p2["id"], p3["id"]}
    assert len(ids) == 3


# =============================================================================
# Starting the game
# =============================================================================


async def test_start_game(service_client, quiz_pack):
    """Starting a game transitions state from 'waiting' to 'active'."""
    game = await create_game(service_client, quiz_pack["pack_id"])
    await add_player(service_client, game["id"], "Alice")

    started = await start_game(service_client, game["id"])

    assert started["id"] == game["id"]
    assert started["state"] == "active"
    assert started["current_question_index"] == 0


# =============================================================================
# Game state (current question)
# =============================================================================


async def test_get_game_state_returns_question_with_variants(service_client, quiz_pack):
    """After starting, game state returns the current question and its variants."""
    game = await create_game(service_client, quiz_pack["pack_id"])
    await add_player(service_client, game["id"], "Alice")
    await start_game(service_client, game["id"])

    state = await get_game_state(service_client, game["id"])

    assert "question" in state
    assert "id" in state["question"]
    assert "text" in state["question"]
    assert "variants" in state
    assert len(state["variants"]) == 4  # Each question has 4 variants

    # The question should be one of the questions in our pack
    q_id = state["question"]["id"]
    assert q_id in quiz_pack["questions_by_id"]


async def test_game_state_does_not_expose_correct_answer(service_client, quiz_pack):
    """Game state should never expose which variant is correct."""
    game = await create_game(service_client, quiz_pack["pack_id"])
    await add_player(service_client, game["id"], "Alice")
    await start_game(service_client, game["id"])

    state = await get_game_state(service_client, game["id"])

    for variant in state["variants"]:
        assert "id" in variant
        assert "text" in variant
        assert "is_correct" not in variant, \
            "is_correct must not be exposed to clients"


# =============================================================================
# Submitting answers
# =============================================================================


async def test_correct_answer_returns_correct_result(service_client, quiz_pack):
    """Submitting a correct answer returns result='correct'."""
    game = await create_game(service_client, quiz_pack["pack_id"])
    alice = await add_player(service_client, game["id"], "Alice")
    # Add a second player so the game doesn't auto-advance after Alice's answer
    await add_player(service_client, game["id"], "Bob")
    await start_game(service_client, game["id"])

    state = await get_game_state(service_client, game["id"])
    q_id = state["question"]["id"]
    q_data = quiz_pack["questions_by_id"][q_id]

    result = await submit_answer(
        service_client, game["id"],
        alice["id"], variant_id=q_data["correct_variant_id"]
    )

    assert result["result"] == "correct"
    assert result["game_finished"] == False


async def test_incorrect_answer_returns_incorrect_result(service_client, quiz_pack):
    """Submitting an incorrect answer returns result='incorrect'."""
    game = await create_game(service_client, quiz_pack["pack_id"])
    alice = await add_player(service_client, game["id"], "Alice")
    await add_player(service_client, game["id"], "Bob")
    await start_game(service_client, game["id"])

    state = await get_game_state(service_client, game["id"])
    q_id = state["question"]["id"]
    q_data = quiz_pack["questions_by_id"][q_id]

    result = await submit_answer(
        service_client, game["id"],
        alice["id"], variant_id=q_data["incorrect_variant_id"]
    )

    assert result["result"] == "incorrect"
    assert result["game_finished"] == False


# =============================================================================
# Complete game flows
# =============================================================================


async def test_single_player_full_game(service_client, quiz_pack):
    """A single player can play through all questions to completion."""
    game = await create_game(service_client, quiz_pack["pack_id"])
    player = await add_player(service_client, game["id"], "Alice")
    await start_game(service_client, game["id"])

    num_questions = quiz_pack["num_questions"]

    for i in range(num_questions):
        state = await get_game_state(service_client, game["id"])
        q_id = state["question"]["id"]
        q_data = quiz_pack["questions_by_id"][q_id]

        result = await submit_answer(
            service_client, game["id"],
            player["id"], variant_id=q_data["correct_variant_id"]
        )

        if i < num_questions - 1:
            # Not the last question - game continues
            assert result["game_finished"] == False
            assert result["result"] == "correct"
        else:
            # Last question - game finishes
            assert result["game_finished"] == True
            assert result["result"] == "game_finished"

    # Verify final results
    results = await get_game_results(service_client, game["id"])
    assert results["game_session"]["state"] == "finished"
    assert results["players"][0]["name"] == "Alice"
    assert results["players"][0]["score"] == num_questions


async def test_multiplayer_full_game(service_client, quiz_pack):
    """Multiple players can complete a game together, scores tracked independently."""
    game = await create_game(service_client, quiz_pack["pack_id"])
    alice = await add_player(service_client, game["id"], "Alice")
    bob = await add_player(service_client, game["id"], "Bob")
    await start_game(service_client, game["id"])

    num_questions = quiz_pack["num_questions"]

    for i in range(num_questions):
        state = await get_game_state(service_client, game["id"])
        q_id = state["question"]["id"]
        q_data = quiz_pack["questions_by_id"][q_id]

        # Alice always answers correctly
        await submit_answer(
            service_client, game["id"],
            alice["id"], variant_id=q_data["correct_variant_id"]
        )

        # Bob always answers incorrectly (his answer triggers advancement)
        result = await submit_answer(
            service_client, game["id"],
            bob["id"], variant_id=q_data["incorrect_variant_id"]
        )

        if i < num_questions - 1:
            assert result["game_finished"] == False
        else:
            assert result["game_finished"] == True

    results = await get_game_results(service_client, game["id"])
    assert results["game_session"]["state"] == "finished"

    players = {p["name"]: p for p in results["players"]}
    assert players["Alice"]["score"] == num_questions
    assert players["Bob"]["score"] == 0


async def test_game_results_contain_all_players(service_client, single_question_pack):
    """Game results include all players who participated."""
    pack = single_question_pack
    game = await create_game(service_client, pack["pack_id"])
    p1 = await add_player(service_client, game["id"], "Alice")
    p2 = await add_player(service_client, game["id"], "Bob")
    p3 = await add_player(service_client, game["id"], "Charlie")
    await start_game(service_client, game["id"])

    # All players answer
    for p in [p1, p2, p3]:
        await submit_answer(
            service_client, game["id"],
            p["id"], variant_id=pack["correct_variant_id"]
        )

    results = await get_game_results(service_client, game["id"])
    assert results["game_session"]["state"] == "finished"
    assert len(results["players"]) == 3

    player_names = {p["name"] for p in results["players"]}
    assert player_names == {"Alice", "Bob", "Charlie"}


async def test_game_results_structure(service_client, single_question_pack):
    """Game results response has the expected structure."""
    pack = single_question_pack
    game = await create_game(service_client, pack["pack_id"])
    player = await add_player(service_client, game["id"], "Alice")
    await start_game(service_client, game["id"])

    await submit_answer(
        service_client, game["id"],
        player["id"], variant_id=pack["correct_variant_id"]
    )

    results = await get_game_results(service_client, game["id"])

    # Verify game_session fields
    assert "game_session" in results
    assert "id" in results["game_session"]
    assert "state" in results["game_session"]
    assert "current_question_index" in results["game_session"]

    # Verify players array fields
    assert "players" in results
    assert len(results["players"]) >= 1
    for p in results["players"]:
        assert "id" in p
        assert "name" in p
        assert "score" in p
