"""Helper functions for game API HTTP endpoints."""

import json
from typing import Dict, Any, Optional


async def create_game(service_client, pack_id: str) -> Dict[str, Any]:
    """Create a new game session. Returns response JSON."""
    response = await service_client.post(
        '/games',
        data=json.dumps({'pack_id': pack_id}),
    )
    assert response.status == 200, f"Failed to create game: {response.text}"
    return response.json()


async def add_player(service_client, game_id: str, player_name: str) -> Dict[str, Any]:
    """Add a player to a game session. Returns response JSON."""
    response = await service_client.post(
        f'/games/{game_id}/players',
        data=json.dumps({'player_name': player_name}),
    )
    assert response.status == 200, f"Failed to add player: {response.text}"
    return response.json()


async def start_game(service_client, game_id: str) -> Dict[str, Any]:
    """Start a game session. Returns response JSON."""
    response = await service_client.post(f'/games/{game_id}/start')
    assert response.status == 200, f"Failed to start game: {response.text}"
    return response.json()


async def get_game_state(service_client, game_id: str) -> Dict[str, Any]:
    """Get the current game state (current question + variants). Returns response JSON."""
    response = await service_client.get(f'/games/{game_id}/state')
    assert response.status == 200, f"Failed to get game state: {response.text}"
    return response.json()


async def submit_answer(
    service_client,
    game_id: str,
    player_id: str,
    variant_id: Optional[str] = None,
    text_answer: Optional[str] = None,
) -> Dict[str, Any]:
    """Submit a player's answer. Returns response JSON."""
    body: Dict[str, Any] = {'player_id': player_id}
    if variant_id is not None:
        body['variant_id'] = variant_id
    if text_answer is not None:
        body['text_answer'] = text_answer
    response = await service_client.post(
        f'/games/{game_id}/answers',
        data=json.dumps(body),
    )
    assert response.status == 200, f"Failed to submit answer: {response.text}"
    return response.json()


async def get_game_results(service_client, game_id: str) -> Dict[str, Any]:
    """Get game results. Returns response JSON."""
    response = await service_client.get(f'/games/{game_id}/results')
    assert response.status == 200, f"Failed to get game results: {response.text}"
    return response.json()
