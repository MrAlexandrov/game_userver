from utils import Routes
from validators import validate_uuid
from typing import Dict, Any, List


async def create_pack(service_client, title: str) -> Dict[str, Any]:
    response = await service_client.post(Routes.CREATE_PACK, params={'title': title})
    assert response.status == 200
    json_response = response.json()

    assert "id" in json_response
    assert json_response["id"] is not None
    assert validate_uuid(json_response["id"])
    assert "title" in json_response
    assert json_response["title"] is not None
    assert json_response["title"] == title

    return json_response


async def get_pack(service_client, uuid: str) -> Dict[str, Any]:
    response = await service_client.get(Routes.GET_PACK, params={'uuid': uuid})
    json_response = response.json()

    return json_response


async def get_all_packs(service_client) -> List[Dict[str, Any]]:
    response = await service_client.get(Routes.GET_ALL_PACKS)
    json_response = response.json()

    return json_response
