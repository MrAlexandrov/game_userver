from utils import Routes
from validators import validate_uuid
from typing import Dict, Any, List


async def create_pack(service_client, title: str) -> Dict[str, Any]:
    response = await service_client.post(Routes.CREATE_PACK, params={'title': title})
    assert response.status == 200
    response_json = response.json()

    assert "id" in response_json
    assert validate_uuid(response_json["id"])

    expected_json = {
        "id": response_json["id"],
        "title": title
    }

    assert response_json == expected_json

    return response_json


async def get_pack(service_client, uuid: str) -> Dict[str, Any]:
    response = await service_client.get(Routes.GET_PACK, params={'uuid': uuid})
    response_json = response.json()

    return response_json


async def get_all_packs(service_client) -> List[Dict[str, Any]]:
    response = await service_client.get(Routes.GET_ALL_PACKS)
    assert response.status == 200
    response_json = response.json()

    return response_json


# ------------------------------------------------------------------------------


async def create_question(service_client, pack_id: str, text: str, image_url: str = "") -> Dict[str, Any]:
    responce = await service_client.post(
        Routes.CREATE_QUESTION,
        params={
            'pack_id': pack_id,
            'text': text,
            'image_url': image_url
        }
    )
    assert responce.status == 200

    response_json = responce.json()

    assert "id" in response_json
    expected_json = {
        "id": response_json["id"],
        "pack_id": pack_id,
        "text": text,
        "image_url": image_url if image_url else ""
    }
    assert response_json == expected_json

    return response_json


async def get_question_by_id(service_client, id: str) -> Dict[str, Any]:
    responce = await service_client.get(Routes.GET_QUESTION_BY_ID, params={'id': id})
    assert responce.status == 200

    return responce.json()


async def get_questions_by_pack_id(service_client, pack_id: str) -> Dict[str, Any]:
    responce = await service_client.get(Routes.GET_QUESTIONS_BY_PACK_ID, params={'pack_id': pack_id})
    assert responce.status == 200

    return responce.json()


# ------------------------------------------------------------------------------


async def create_variant(service_client, question_id: str, text: str, is_correct: bool = False) -> Dict[str, Any]:
    responce = await service_client.post(
        Routes.CREATE_VARIANT,
        params={
            'question_id': question_id,
            'text': text,
            'is_correct': is_correct
        }
    )
    assert responce.status == 200

    response_json = responce.json()

    assert "id" in response_json
    expected_json = {
        "id": response_json["id"],
        "question_id": question_id,
        "text": text,
        "is_correct": is_correct
    }
    assert response_json == expected_json

    return response_json


async def get_variant_by_id(service_client, id: str) -> Dict[str, Any]:
    responce = await service_client.get(Routes.GET_VARIANT_BY_ID, params={'id': id})
    assert responce.status == 200

    return responce.json()


async def get_variants_by_question_id(service_client, question_id: str) -> Dict[str, Any]:
    responce = await service_client.get(Routes.GET_VARIANTS_BY_QUESTION_ID, params={'question_id': question_id})
    assert responce.status == 200

    return responce.json()
