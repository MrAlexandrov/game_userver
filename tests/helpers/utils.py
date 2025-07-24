from enum import Enum

class Routes(str, Enum):
    CREATE_PACK         = "/create-pack"
    GET_PACK            = "/get-pack"
    GET_ALL_PACKS       = "/get-all-packs"

    def __str__(self) -> str:
        return self.value
