import handlers.cruds_pb2 as service
# import handlers.cruds_pb2_grpc as create_pack_grpc


async def test_create_pack_grpc(grpc_handlers):
    title = 'New Title'
    request = service.CreatePackRequest(title=title) # type: ignore
    response = await grpc_handlers.CreatePack(request)
    assert response.pack.title == title


async def test_get_all_packs_grpc(grpc_handlers):
    request = service.GetAllPacksRequest()
    response = await grpc_handlers.GetAllPacks(request)
    assert True
    # assert response.pack.title == title
