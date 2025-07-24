from uuid import UUID

def validate_uuid(uuid_to_test: str, version=4) -> bool:
    try:
        UUID(uuid_to_test, version=version)
    except ValueError:
        return False
    return True
