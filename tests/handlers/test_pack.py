import pytest
from endpoints import (
    create_pack,
    get_pack
)


async def test_create_pack(service_client):
    await create_pack(service_client, "some_title")
    assert True


async def test_get_pack(service_client):
    created_pack = await create_pack(service_client, "test_title")
    getted_pack = await get_pack(service_client, created_pack["id"])
    assert created_pack == getted_pack
