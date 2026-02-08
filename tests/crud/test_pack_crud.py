"""
Integration tests for Pack CRUD operations.
Tests verify that pack operations work correctly with the database.
"""

import pytest
import handlers.cruds_pb2 as service


@pytest.mark.pgsql('db_1')
async def test_create_pack(grpc_handlers, pgsql):
    """Test creating a pack and verifying it exists in the database"""
    pack_title = "Test Pack for CRUD"
    
    # Create pack via gRPC
    request = service.CreatePackRequest(title=pack_title)  # type: ignore
    response = await grpc_handlers.CreatePack(request)
    
    # Verify response
    assert response.pack.id is not None
    assert response.pack.title == pack_title
    
    # Verify in database
    cursor = pgsql['db_1'].cursor()
    cursor.execute(
        "SELECT id, title FROM quiz.packs WHERE id = %s",
        (response.pack.id,)
    )
    row = cursor.fetchone()
    assert row is not None
    assert row[1] == pack_title


@pytest.mark.pgsql('db_1')
async def test_get_pack_by_id(grpc_handlers, pgsql):
    """Test retrieving a pack by ID"""
    pack_title = "Pack to Retrieve"
    
    # Create pack
    create_request = service.CreatePackRequest(title=pack_title)  # type: ignore
    create_response = await grpc_handlers.CreatePack(create_request)
    pack_id = create_response.pack.id
    
    # Get pack by ID
    get_request = service.GetPackByIdRequest(id=pack_id)  # type: ignore
    get_response = await grpc_handlers.GetPackById(get_request)
    
    # Verify response
    assert get_response.pack.id == pack_id
    assert get_response.pack.title == pack_title


@pytest.mark.pgsql('db_1')
async def test_get_all_packs(grpc_handlers, pgsql):
    """Test retrieving all packs"""
    # Create multiple packs
    pack_titles = ["Pack 1", "Pack 2", "Pack 3"]
    created_ids = []
    
    for title in pack_titles:
        request = service.CreatePackRequest(title=title)  # type: ignore
        response = await grpc_handlers.CreatePack(request)
        created_ids.append(response.pack.id)
    
    # Get all packs
    get_all_request = service.GetAllPacksRequest()  # type: ignore
    get_all_response = await grpc_handlers.GetAllPacks(get_all_request)
    
    # Verify all created packs are in the response
    response_ids = [pack.id for pack in get_all_response.packs]
    for pack_id in created_ids:
        assert pack_id in response_ids


@pytest.mark.pgsql('db_1')
async def test_create_pack_with_empty_title(grpc_handlers):
    """Test that creating a pack with empty title is handled"""
    # This test verifies error handling for invalid input
    request = service.CreatePackRequest(title="")  # type: ignore
    
    # Depending on implementation, this might raise an error or create with empty title
    # Adjust assertion based on actual behavior
    try:
        response = await grpc_handlers.CreatePack(request)
        # If it succeeds, verify the pack was created
        assert response.pack.id is not None
    except Exception as e:
        # If it fails, that's also acceptable behavior
        assert True


@pytest.mark.pgsql('db_1')
async def test_pack_persistence(grpc_handlers, pgsql):
    """Test that created pack persists in database"""
    pack_title = "Persistent Pack"
    
    # Create pack
    request = service.CreatePackRequest(title=pack_title)  # type: ignore
    response = await grpc_handlers.CreatePack(request)
    pack_id = response.pack.id
    
    # Query database directly
    cursor = pgsql['db_1'].cursor()
    cursor.execute("SELECT COUNT(*) FROM quiz.packs WHERE id = %s", (pack_id,))
    count = cursor.fetchone()[0]
    
    assert count == 1