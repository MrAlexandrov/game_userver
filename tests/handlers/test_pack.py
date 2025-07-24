import pytest
from endpoints import (
    create_pack,
    get_all_packs,
    get_pack
)


async def test_create_pack(service_client):
    await create_pack(service_client, "some_title")
    assert True


async def test_get_pack(service_client):
    created_pack = await create_pack(service_client, "test_title")
    getted_pack = await get_pack(service_client, created_pack["id"])
    assert created_pack == getted_pack


async def test_get_all_packs(service_client):
    second_pack = await create_pack(service_client, "second_pack")
    first_pack = await create_pack(service_client, "first_pack")

    all_packs = await get_all_packs(service_client)

    assert len(all_packs) == 2
    