"""Fixtures for game integration tests.

Creates quiz packs with questions and variants via gRPC,
ready to be used in game flow tests via HTTP.
"""

import pytest
import handlers.cruds_pb2 as service


@pytest.fixture
async def quiz_pack(grpc_handlers):
    """
    Create a pack with 3 multiple-choice questions, each having 4 variants (1 correct).

    Returns dict with:
        - pack_id: str
        - questions_by_id: dict mapping question_id -> {id, text, correct_variant_id, incorrect_variant_id}
        - num_questions: int
    """
    pack_resp = await grpc_handlers.CreatePack(
        service.CreatePackRequest(title="Integration Test Quiz")  # type: ignore
    )
    pack_id = pack_resp.pack.id

    questions_data = [
        ("What is 2 + 2?", [("4", True), ("3", False), ("5", False), ("6", False)]),
        ("Capital of France?", [("Paris", True), ("London", False), ("Berlin", False), ("Rome", False)]),
        ("Closest planet to Sun?", [("Mercury", True), ("Venus", False), ("Earth", False), ("Mars", False)]),
    ]

    questions_by_id = {}

    for q_text, variants_data in questions_data:
        q_resp = await grpc_handlers.CreateQuestion(
            service.CreateQuestionRequest(
                pack_id=pack_id,  # type: ignore
                text=q_text,  # type: ignore
                image_url=""  # type: ignore
            )
        )
        q_id = q_resp.question.id

        correct_variant_id = None
        incorrect_variant_id = None

        for v_text, v_correct in variants_data:
            v_resp = await grpc_handlers.CreateVariant(
                service.CreateVariantRequest(
                    question_id=q_id,  # type: ignore
                    text=v_text,  # type: ignore
                    is_correct=v_correct  # type: ignore
                )
            )
            if v_correct:
                correct_variant_id = v_resp.variant.id
            elif incorrect_variant_id is None:
                incorrect_variant_id = v_resp.variant.id

        questions_by_id[q_id] = {
            "id": q_id,
            "text": q_text,
            "correct_variant_id": correct_variant_id,
            "incorrect_variant_id": incorrect_variant_id,
        }

    return {
        "pack_id": pack_id,
        "questions_by_id": questions_by_id,
        "num_questions": len(questions_data),
    }


@pytest.fixture
async def single_question_pack(grpc_handlers):
    """
    Create a pack with a single multiple-choice question and 4 variants.

    Returns dict with:
        - pack_id: str
        - question_id: str
        - correct_variant_id: str
        - incorrect_variant_id: str
    """
    pack_resp = await grpc_handlers.CreatePack(
        service.CreatePackRequest(title="Single Question Quiz")  # type: ignore
    )
    pack_id = pack_resp.pack.id

    q_resp = await grpc_handlers.CreateQuestion(
        service.CreateQuestionRequest(
            pack_id=pack_id,  # type: ignore
            text="What is 1 + 1?",  # type: ignore
            image_url=""  # type: ignore
        )
    )
    question_id = q_resp.question.id

    correct_variant_id = None
    incorrect_variant_id = None

    for v_text, v_correct in [("2", True), ("3", False), ("0", False), ("1", False)]:
        v_resp = await grpc_handlers.CreateVariant(
            service.CreateVariantRequest(
                question_id=question_id,  # type: ignore
                text=v_text,  # type: ignore
                is_correct=v_correct  # type: ignore
            )
        )
        if v_correct:
            correct_variant_id = v_resp.variant.id
        elif incorrect_variant_id is None:
            incorrect_variant_id = v_resp.variant.id

    return {
        "pack_id": pack_id,
        "question_id": question_id,
        "correct_variant_id": correct_variant_id,
        "incorrect_variant_id": incorrect_variant_id,
    }
