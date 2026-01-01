import pathlib
import sys
import os

import handlers.hello_pb2_grpc as hello_services
import handlers.cruds_pb2_grpc as grpc_handlers_service
import handlers.cruds_pb2 as service
import pytest

from testsuite.databases.pgsql import discover

pytest_plugins = [
    'pytest_userver.plugins.core',
    'pytest_userver.plugins.postgresql',
    'pytest_userver.plugins.grpc',
]
USERVER_CONFIG_HOOKS = [
    # 'prepare_service_config_grpc',
]


# Add tests directory to sys.path so helpers can be imported as a package
sys.path.insert(0, os.path.dirname(__file__))


@pytest.fixture
def grpc_service(grpc_channel, service_client):
    return hello_services.HelloServiceStub(grpc_channel)


@pytest.fixture
def grpc_handlers(grpc_channel, service_client):
    return grpc_handlers_service.QuizServiceStub(grpc_channel)


@pytest.fixture
def sample_pack_title():
    return "Science Quiz"


@pytest.fixture
def sample_question_text():
    return "What is the capital of France?"


@pytest.fixture
def sample_image_url():
    return "http://example.com/paris.jpg"


@pytest.fixture
def sample_variant_data():
    return [
        ("London", False),
        ("Berlin", False),
        ("Paris", True),
        ("Madrid", False),
    ]


@pytest.fixture
async def created_pack_id(grpc_handlers, sample_pack_title):
    """Создаёт пак и возвращает его ID"""
    request = service.CreatePackRequest(title=sample_pack_title) # type: ignore
    response = await grpc_handlers.CreatePack(request)
    return response.pack.id


@pytest.fixture
async def created_question_id(grpc_handlers, created_pack_id, sample_question_text, sample_image_url):
    """Создаёт вопрос в паке и возвращает ID"""
    request = service.CreateQuestionRequest(
        pack_id=created_pack_id, # type: ignore
        text=sample_question_text, # type: ignore
        image_url=sample_image_url # type: ignore
    )
    response = await grpc_handlers.CreateQuestion(request)
    return response.question.id


@pytest.fixture
async def created_variants_ids(grpc_handlers, created_question_id, sample_variant_data):
    """Создаёт несколько вариантов для вопроса и возвращает их ID"""
    variant_ids = []
    for text, is_correct in sample_variant_data:
        request = service.CreateVariantRequest(
            question_id=created_question_id, # type: ignore
            text=text, # type: ignore
            is_correct=is_correct # type: ignore
        )
        response = await grpc_handlers.CreateVariant(request)
        variant_ids.append(response.variant.id)
    return variant_ids



def pytest_configure(config):
    sys.path.append(str(pathlib.Path(__file__).parent.parent / 'proto/handlers/'))




@pytest.fixture(scope='session')
def service_source_dir():
    """Path to root directory service."""
    return pathlib.Path(__file__).parent.parent


@pytest.fixture(scope='session')
def initial_data_path(service_source_dir):
    """Path for find files with data"""
    return [
        service_source_dir / 'postgresql/data',
    ]


@pytest.fixture(scope='session')
def pgsql_local(service_source_dir, pgsql_local_create):
    """Create schemas databases for tests"""
    databases = discover.find_schemas(
        'game_userver',
        [service_source_dir.joinpath('postgresql/schemas')],
    )
    return pgsql_local_create(list(databases.values()))
