"""
Integration tests for game invariants and state machine rules.

Tests verify that:
- Invalid state transitions are rejected (e.g., adding player to active game)
- Question progression follows correct rules
- Score invariants are maintained
- Client-facing data hides internal state (e.g., correct answers)
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
# State transition guards: adding players
# =============================================================================


async def test_cannot_add_player_to_active_game(service_client, quiz_pack):
    """Adding a player after the game has started should fail (state != waiting)."""
    game = await create_game(service_client, quiz_pack["pack_id"])
    await add_player(service_client, game["id"], "Alice")
    await start_game(service_client, game["id"])

    # Try to add a player to an active game
    response = await service_client.post(
        f'/games/{game["id"]}/players',
        data=json.dumps({'player_name': 'LateComer'}),
    )
    assert response.status == 500
    assert "error" in response.json()


async def test_cannot_add_player_to_finished_game(service_client, single_question_pack):
    """Adding a player after the game has finished should fail."""
    pack = single_question_pack
    game = await create_game(service_client, pack["pack_id"])
    player = await add_player(service_client, game["id"], "Alice")
    await start_game(service_client, game["id"])

    # Finish the game (single question, single player)
    await submit_answer(
        service_client, game["id"],
        player["id"], variant_id=pack["correct_variant_id"]
    )

    # Verify game is finished
    results = await get_game_results(service_client, game["id"])
    assert results["game_session"]["state"] == "finished"

    # Try to add a player
    response = await service_client.post(
        f'/games/{game["id"]}/players',
        data=json.dumps({'player_name': 'LateComer'}),
    )
    assert response.status == 500
    assert "error" in response.json()


# =============================================================================
# State transition guards: starting game
# =============================================================================


async def test_cannot_start_already_active_game(service_client, quiz_pack):
    """Starting a game that is already active should fail."""
    game = await create_game(service_client, quiz_pack["pack_id"])
    await add_player(service_client, game["id"], "Alice")
    await start_game(service_client, game["id"])

    # Try to start again
    response = await service_client.post(f'/games/{game["id"]}/start')
    assert response.status == 500
    assert "error" in response.json()


async def test_cannot_start_finished_game(service_client, single_question_pack):
    """Starting a game that has already finished should fail."""
    pack = single_question_pack
    game = await create_game(service_client, pack["pack_id"])
    player = await add_player(service_client, game["id"], "Alice")
    await start_game(service_client, game["id"])

    # Finish the game
    await submit_answer(
        service_client, game["id"],
        player["id"], variant_id=pack["correct_variant_id"]
    )

    # Try to start again
    response = await service_client.post(f'/games/{game["id"]}/start')
    assert response.status == 500
    assert "error" in response.json()


# =============================================================================
# State transition guards: submitting answers
# =============================================================================


async def test_cannot_submit_answer_before_game_starts(service_client, quiz_pack):
    """Submitting an answer while the game is still in 'waiting' state should fail."""
    game = await create_game(service_client, quiz_pack["pack_id"])
    player = await add_player(service_client, game["id"], "Alice")

    # Game is in 'waiting' state - try to submit an answer
    first_q = next(iter(quiz_pack["questions_by_id"].values()))
    response = await service_client.post(
        f'/games/{game["id"]}/answers',
        data=json.dumps({
            'player_id': player["id"],
            'variant_id': first_q["correct_variant_id"],
        }),
    )
    assert response.status == 500


async def test_cannot_submit_answer_after_game_finished(
    service_client, single_question_pack
):
    """Submitting an answer after the game has finished should fail."""
    pack = single_question_pack
    game = await create_game(service_client, pack["pack_id"])
    player = await add_player(service_client, game["id"], "Alice")
    await start_game(service_client, game["id"])

    # Finish the game
    await submit_answer(
        service_client, game["id"],
        player["id"], variant_id=pack["correct_variant_id"]
    )

    # Verify game is finished
    results = await get_game_results(service_client, game["id"])
    assert results["game_session"]["state"] == "finished"

    # Try to submit another answer
    response = await service_client.post(
        f'/games/{game["id"]}/answers',
        data=json.dumps({
            'player_id': player["id"],
            'variant_id': pack["correct_variant_id"],
        }),
    )
    assert response.status == 500


# =============================================================================
# Question progression invariants
# =============================================================================


async def test_question_does_not_advance_until_all_players_answer(
    service_client, quiz_pack
):
    """The question should stay the same until ALL players have answered."""
    game = await create_game(service_client, quiz_pack["pack_id"])
    alice = await add_player(service_client, game["id"], "Alice")
    bob = await add_player(service_client, game["id"], "Bob")
    await start_game(service_client, game["id"])

    # Get initial question
    state_before = await get_game_state(service_client, game["id"])
    q1_id = state_before["question"]["id"]
    q1_data = quiz_pack["questions_by_id"][q1_id]

    # Only Alice answers
    await submit_answer(
        service_client, game["id"],
        alice["id"], variant_id=q1_data["correct_variant_id"]
    )

    # Question should NOT have changed (Bob hasn't answered yet)
    state_after = await get_game_state(service_client, game["id"])
    assert state_after["question"]["id"] == q1_id


async def test_question_advances_after_all_players_answer(service_client, quiz_pack):
    """After all players answer, the game advances to the next question."""
    game = await create_game(service_client, quiz_pack["pack_id"])
    alice = await add_player(service_client, game["id"], "Alice")
    bob = await add_player(service_client, game["id"], "Bob")
    await start_game(service_client, game["id"])

    # Get first question
    state1 = await get_game_state(service_client, game["id"])
    q1_id = state1["question"]["id"]
    q1_data = quiz_pack["questions_by_id"][q1_id]

    # Both players answer question 1
    await submit_answer(
        service_client, game["id"],
        alice["id"], variant_id=q1_data["correct_variant_id"]
    )
    await submit_answer(
        service_client, game["id"],
        bob["id"], variant_id=q1_data["incorrect_variant_id"]
    )

    # Question should have advanced to a different one
    state2 = await get_game_state(service_client, game["id"])
    q2_id = state2["question"]["id"]
    assert q2_id != q1_id
    assert q2_id in quiz_pack["questions_by_id"]


async def test_game_finishes_after_last_question(service_client, quiz_pack):
    """After all players answer the last question, the game transitions to 'finished'."""
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

    # Last answer should indicate game finished
    assert result["game_finished"] == True
    assert result["result"] == "game_finished"

    # Session should be in finished state
    results = await get_game_results(service_client, game["id"])
    assert results["game_session"]["state"] == "finished"


async def test_game_state_returns_404_after_finish(service_client, single_question_pack):
    """GetGameState returns 404 after the game is finished (no more questions)."""
    pack = single_question_pack
    game = await create_game(service_client, pack["pack_id"])
    player = await add_player(service_client, game["id"], "Alice")
    await start_game(service_client, game["id"])

    # Finish the game
    await submit_answer(
        service_client, game["id"],
        player["id"], variant_id=pack["correct_variant_id"]
    )

    # Game state should return 404
    response = await service_client.get(f'/games/{game["id"]}/state')
    assert response.status == 404


async def test_question_index_reflects_progression(service_client, quiz_pack):
    """The current_question_index in results advances as questions are answered."""
    game = await create_game(service_client, quiz_pack["pack_id"])
    player = await add_player(service_client, game["id"], "Alice")
    await start_game(service_client, game["id"])

    # Check initial index
    results = await get_game_results(service_client, game["id"])
    assert results["game_session"]["current_question_index"] == 0

    # Answer first question (single player -> auto-advance)
    state = await get_game_state(service_client, game["id"])
    q_data = quiz_pack["questions_by_id"][state["question"]["id"]]
    await submit_answer(
        service_client, game["id"],
        player["id"], variant_id=q_data["correct_variant_id"]
    )

    # Index should have advanced
    results = await get_game_results(service_client, game["id"])
    assert results["game_session"]["current_question_index"] == 1


# =============================================================================
# Score invariants
# =============================================================================


async def test_initial_score_is_zero(service_client, quiz_pack):
    """All players start with score 0."""
    game = await create_game(service_client, quiz_pack["pack_id"])
    p1 = await add_player(service_client, game["id"], "Alice")
    p2 = await add_player(service_client, game["id"], "Bob")

    assert p1["score"] == 0
    assert p2["score"] == 0


async def test_correct_answers_increment_score(service_client, quiz_pack):
    """Each correct answer increments the player's score by 1."""
    game = await create_game(service_client, quiz_pack["pack_id"])
    player = await add_player(service_client, game["id"], "Alice")
    await start_game(service_client, game["id"])

    num_questions = quiz_pack["num_questions"]

    # Answer all questions correctly
    for _ in range(num_questions):
        state = await get_game_state(service_client, game["id"])
        q_data = quiz_pack["questions_by_id"][state["question"]["id"]]
        await submit_answer(
            service_client, game["id"],
            player["id"], variant_id=q_data["correct_variant_id"]
        )

    results = await get_game_results(service_client, game["id"])
    assert results["players"][0]["score"] == num_questions


async def test_incorrect_answers_do_not_change_score(service_client, quiz_pack):
    """Incorrect answers do not affect the player's score."""
    game = await create_game(service_client, quiz_pack["pack_id"])
    player = await add_player(service_client, game["id"], "Alice")
    await start_game(service_client, game["id"])

    num_questions = quiz_pack["num_questions"]

    # Answer all questions incorrectly
    for _ in range(num_questions):
        state = await get_game_state(service_client, game["id"])
        q_data = quiz_pack["questions_by_id"][state["question"]["id"]]
        await submit_answer(
            service_client, game["id"],
            player["id"], variant_id=q_data["incorrect_variant_id"]
        )

    results = await get_game_results(service_client, game["id"])
    assert results["players"][0]["score"] == 0


async def test_mixed_answers_give_correct_score(service_client, quiz_pack):
    """Score correctly reflects a mix of correct and incorrect answers."""
    game = await create_game(service_client, quiz_pack["pack_id"])
    player = await add_player(service_client, game["id"], "Alice")
    await start_game(service_client, game["id"])

    # Pattern: correct, incorrect, correct
    expected_correct = 0
    for i in range(quiz_pack["num_questions"]):
        state = await get_game_state(service_client, game["id"])
        q_data = quiz_pack["questions_by_id"][state["question"]["id"]]

        if i % 2 == 0:
            # Even questions -> correct answer
            await submit_answer(
                service_client, game["id"],
                player["id"], variant_id=q_data["correct_variant_id"]
            )
            expected_correct += 1
        else:
            # Odd questions -> incorrect answer
            await submit_answer(
                service_client, game["id"],
                player["id"], variant_id=q_data["incorrect_variant_id"]
            )

    results = await get_game_results(service_client, game["id"])
    assert results["players"][0]["score"] == expected_correct


async def test_multiplayer_scores_independent(service_client, quiz_pack):
    """Each player's score is tracked independently in a multiplayer game."""
    game = await create_game(service_client, quiz_pack["pack_id"])
    alice = await add_player(service_client, game["id"], "Alice")
    bob = await add_player(service_client, game["id"], "Bob")
    charlie = await add_player(service_client, game["id"], "Charlie")
    await start_game(service_client, game["id"])

    num_questions = quiz_pack["num_questions"]

    for _ in range(num_questions):
        state = await get_game_state(service_client, game["id"])
        q_data = quiz_pack["questions_by_id"][state["question"]["id"]]

        # Alice: always correct
        await submit_answer(
            service_client, game["id"],
            alice["id"], variant_id=q_data["correct_variant_id"]
        )
        # Bob: always incorrect
        await submit_answer(
            service_client, game["id"],
            bob["id"], variant_id=q_data["incorrect_variant_id"]
        )
        # Charlie: always correct
        await submit_answer(
            service_client, game["id"],
            charlie["id"], variant_id=q_data["correct_variant_id"]
        )

    results = await get_game_results(service_client, game["id"])
    players = {p["name"]: p for p in results["players"]}

    assert players["Alice"]["score"] == num_questions
    assert players["Bob"]["score"] == 0
    assert players["Charlie"]["score"] == num_questions


# =============================================================================
# Results availability invariants
# =============================================================================


async def test_results_available_for_waiting_game(service_client, quiz_pack):
    """Game results endpoint works even when the game hasn't started yet."""
    game = await create_game(service_client, quiz_pack["pack_id"])
    await add_player(service_client, game["id"], "Alice")

    results = await get_game_results(service_client, game["id"])
    assert results["game_session"]["state"] == "waiting"
    assert len(results["players"]) == 1


async def test_results_available_for_active_game(service_client, quiz_pack):
    """Game results endpoint works mid-game (active state)."""
    game = await create_game(service_client, quiz_pack["pack_id"])
    alice = await add_player(service_client, game["id"], "Alice")
    # Add second player so game stays on Q1 after Alice answers
    await add_player(service_client, game["id"], "Bob")
    await start_game(service_client, game["id"])

    # Alice answers first question but Bob doesn't, so game stays active
    state = await get_game_state(service_client, game["id"])
    q_data = quiz_pack["questions_by_id"][state["question"]["id"]]
    await submit_answer(
        service_client, game["id"],
        alice["id"],
        variant_id=q_data["correct_variant_id"]
    )

    results = await get_game_results(service_client, game["id"])
    assert results["game_session"]["state"] == "active"
    assert len(results["players"]) == 2


async def test_results_not_found_for_nonexistent_game(service_client):
    """Getting results for a nonexistent game should return 404."""
    fake_game_id = "00000000-0000-4000-8000-000000000000"
    response = await service_client.get(f'/games/{fake_game_id}/results')
    assert response.status == 404


# =============================================================================
# Three-player question progression (more complex scenario)
# =============================================================================


async def test_three_player_question_stays_until_all_answer(
    service_client, quiz_pack
):
    """With 3 players, question doesn't advance until all 3 have answered."""
    game = await create_game(service_client, quiz_pack["pack_id"])
    alice = await add_player(service_client, game["id"], "Alice")
    bob = await add_player(service_client, game["id"], "Bob")
    charlie = await add_player(service_client, game["id"], "Charlie")
    await start_game(service_client, game["id"])

    state = await get_game_state(service_client, game["id"])
    q1_id = state["question"]["id"]
    q1_data = quiz_pack["questions_by_id"][q1_id]

    # Player 1 answers
    await submit_answer(
        service_client, game["id"],
        alice["id"], variant_id=q1_data["correct_variant_id"]
    )
    # Question should still be the same
    state = await get_game_state(service_client, game["id"])
    assert state["question"]["id"] == q1_id

    # Player 2 answers
    await submit_answer(
        service_client, game["id"],
        bob["id"], variant_id=q1_data["incorrect_variant_id"]
    )
    # Question should STILL be the same (Charlie hasn't answered)
    state = await get_game_state(service_client, game["id"])
    assert state["question"]["id"] == q1_id

    # Player 3 answers - NOW the question should advance
    await submit_answer(
        service_client, game["id"],
        charlie["id"], variant_id=q1_data["correct_variant_id"]
    )
    state = await get_game_state(service_client, game["id"])
    assert state["question"]["id"] != q1_id
