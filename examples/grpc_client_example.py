#!/usr/bin/env python3
"""
Example gRPC client for the quiz service.

This script demonstrates how to interact with the quiz service using gRPC.
"""

import grpc
import sys
import os
import uuid

# Add the generated proto files to the path
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'proto'))

# Import the generated gRPC classes
import handlers.cruds_pb2 as service
import handlers.cruds_service_pb2_grpc as service_grpc

def create_channel(host='localhost', port=8081):
    """Create a gRPC channel to the service."""
    return grpc.insecure_channel(f'{host}:{port}')

def create_pack(stub, title):
    """Create a new quiz pack."""
    request = service.CreatePackRequest(title=title)
    response = stub.CreatePack(request)
    print(f"Created pack: {response.pack.id} - {response.pack.title}")
    return response.pack.id

def get_all_packs(stub):
    """Get all quiz packs."""
    request = service.GetAllPacksRequest()
    response = stub.GetAllPacks(request)
    print(f"Found {len(response.packs)} packs:")
    for pack in response.packs:
        print(f"  {pack.id} - {pack.title}")
    return response.packs

def create_question(stub, pack_id, text, image_url=""):
    """Create a new question in a pack."""
    request = service.CreateQuestionRequest(
        pack_id=pack_id,
        text=text,
        image_url=image_url
    )
    response = stub.CreateQuestion(request)
    print(f"Created question: {response.question.id} - {response.question.text}")
    return response.question.id

def create_variant(stub, question_id, text, is_correct=False):
    """Create a new variant for a question."""
    request = service.CreateVariantRequest(
        question_id=question_id,
        text=text,
        is_correct=is_correct
    )
    response = stub.CreateVariant(request)
    print(f"Created variant: {response.variant.id} - {response.variant.text} (correct: {response.variant.is_correct})")
    return response.variant.id

def create_game_session(stub, pack_id):
    """Create a new game session."""
    request = service.CreateGameSessionRequest(pack_id=pack_id)
    response = stub.CreateGameSession(request)
    print(f"Created game session: {response.game_session.id} (state: {response.game_session.state})")
    return response.game_session.id

def add_player(stub, game_session_id, name):
    """Add a player to a game session."""
    request = service.AddPlayerRequest(
        game_session_id=game_session_id,
        name=name
    )
    response = stub.AddPlayer(request)
    print(f"Added player: {response.player.id} - {response.player.name}")
    return response.player.id

def get_current_question(stub, game_session_id):
    """Get the current question in a game session."""
    request = service.GetCurrentQuestionRequest(game_session_id=game_session_id)
    try:
        response = stub.GetCurrentQuestion(request)
        print(f"Current question: {response.question.text}")
        print("Variants:")
        for i, variant in enumerate(response.variants):
            print(f"  {i+1}. {variant.text} (correct: {variant.is_correct})")
        return response
    except grpc.RpcError as e:
        print(f"Error getting current question: {e.details()}")
        return None

def advance_to_next_question(stub, game_session_id):
    """Advance to the next question in a game session."""
    request = service.AdvanceToNextQuestionRequest(game_session_id=game_session_id)
    try:
        response = stub.AdvanceToNextQuestion(request)
        if response.has_more_questions:
            print("Advanced to next question")
        else:
            print("No more questions")
        return response.has_more_questions
    except grpc.RpcError as e:
        print(f"Error advancing to next question: {e.details()}")
        return False

def submit_answer(stub, player_id, question_id, variant_id):
    """Submit a player's answer."""
    request = service.SubmitAnswerRequest(
        player_id=player_id,
        question_id=question_id,
        variant_id=variant_id
    )
    try:
        response = stub.SubmitAnswer(request)
        print(f"Answer submitted: correct={response.is_correct}, points={response.points}")
        return response.is_correct
    except grpc.RpcError as e:
        print(f"Error submitting answer: {e.details()}")
        return False

def main():
    """Main function demonstrating the gRPC API usage."""
    print("Connecting to quiz service...")
    
    with create_channel() as channel:
        stub = service_grpc.QuizServiceStub(channel)
        
        print("\n1. Creating a quiz pack...")
        pack_id = create_pack(stub, "Sample Quiz Pack")
        
        print("\n2. Creating questions...")
        question1_id = create_question(stub, pack_id, "What is 2+2?")
        question2_id = create_question(stub, pack_id, "What is the capital of France?")
        
        print("\n3. Creating variants for question 1...")
        create_variant(stub, question1_id, "3", False)
        create_variant(stub, question1_id, "4", True)
        create_variant(stub, question1_id, "5", False)
        
        print("\n4. Creating variants for question 2...")
        create_variant(stub, question2_id, "London", False)
        create_variant(stub, question2_id, "Berlin", False)
        create_variant(stub, question2_id, "Paris", True)
        
        print("\n5. Creating a game session...")
        game_session_id = create_game_session(stub, pack_id)
        
        print("\n6. Adding players...")
        player1_id = add_player(stub, game_session_id, "Alice")
        player2_id = add_player(stub, game_session_id, "Bob")
        
        print("\n7. Starting the game session...")
        start_request = service.StartGameSessionRequest(id=game_session_id)
        start_response = stub.StartGameSession(start_request)
        print(f"Game session started: {start_response.game_session.state}")
        
        print("\n8. Getting current question...")
        get_current_question(stub, game_session_id)
        
        print("\n9. Advancing to next question...")
        advance_to_next_question(stub, game_session_id)
        
        print("\n10. Getting new current question...")
        get_current_question(stub, game_session_id)
        
        print("\n11. Getting all packs...")
        get_all_packs(stub)
        
        print("\nDemo completed successfully!")

if __name__ == '__main__':
    main()