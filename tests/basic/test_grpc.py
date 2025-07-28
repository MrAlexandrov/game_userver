# Start the tests via `make test-debug` or `make test-release`

import handlers.hello_pb2 as hello_protos  # noqa: E402, E501
# import handlers.hello_pb2_grpc as hello_protos_grpc  # noqa: E402, E501
import handlers.cruds_pb2 as create_pack
# import handlers.cruds_pb2_grpc as create_pack_grpc
import pytest


async def test_grpc_service(grpc_service):
    request = hello_protos.HelloRequest(name='Tester') # type: ignore
    response = await grpc_service.SayHello(request)
    assert response.text == 'Hello, Tester!\n'


async def test_grpc_create_pack(grpc_handlers):
    title = 'New Title'
    request = create_pack.CreatePackRequest(title=title) # type: ignore
    response = await grpc_handlers.CreatePack(request)
    assert response.pack.title == title
