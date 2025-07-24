from enum import Enum

class Routes(str, Enum):
    CREATE_PACK                     = "/create-pack"
    GET_PACK                        = "/get-pack"
    GET_ALL_PACKS                   = "/get-all-packs"

    CREATE_QUESTION                 = "/create-question"
    GET_QUESTION_BY_ID              = "/get-question-by-id"
    GET_QUESTIONS_BY_PACK_ID        = "/get-questions-by-pack-id"

    CREATE_VARIANT                  = "/create-variant"
    GET_VARIANT_BY_ID               = "/get-variant-by-id"
    GET_VARIANTS_BY_QUESTION_ID     = "/get-variants-by-question-id"


    def __str__(self) -> str:
        return self.value
