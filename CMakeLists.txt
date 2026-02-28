cmake_minimum_required(VERSION 3.12...3.31)
# On Linux use GCC 13; on macOS let CMake auto-detect (Apple Clang)
if(NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
  set(CMAKE_CXX_COMPILER "g++-13")
  set(CMAKE_C_COMPILER "gcc-13")
endif()
project(game_userver CXX)

# Adding userver dependency
find_package(userver COMPONENTS core grpc postgresql QUIET)
if(NOT userver_FOUND)  # Fallback to subdirectory usage
    # Enable userver libraries that are needed in this project
    set(USERVER_FEATURE_GRPC ON CACHE BOOL "" FORCE)
    set(USERVER_FEATURE_POSTGRESQL ON CACHE BOOL "" FORCE)
    
    # Compatibility mode: some systems don't support these features
    set(USERVER_FEATURE_CRYPTOPP_BLAKE2 OFF CACHE BOOL "" FORCE)
    
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/userver)
        message(STATUS "Using userver framework from third_party/userver")
        add_subdirectory(third_party/userver)
    else()
        message(FATAL_ERROR "Either install the userver or provide a path to it")
    endif()
endif()

# On macOS: apply patch to fix missing absl includes in gRPC 1.59.1 CFStream backend.
# gRPC cfstream_endpoint.cc and dns_service_resolver.cc are missing absl/strings includes
# which are required with Apple Clang 16+.
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    set(_grpc_cfstream_dir "${CMAKE_BINARY_DIR}/_deps/grpc-src/src/core/lib/event_engine/cf_engine")
    if(EXISTS "${_grpc_cfstream_dir}/cfstream_endpoint.cc")
        execute_process(
            COMMAND patch --forward --reject-file=- -p1
                    -i "${CMAKE_CURRENT_SOURCE_DIR}/cmake/grpc_macos_cfstream_fix.patch"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/_deps/grpc-src"
            RESULT_VARIABLE _patch_result
            OUTPUT_QUIET ERROR_QUIET
        )
        if(_patch_result EQUAL 0)
            message(STATUS "Applied gRPC macOS CFStream patch")
        endif()
    endif()
endif()

userver_setup_environment()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)

# Common sources
include_directories(src)

userver_add_sql_library(
    ${PROJECT_NAME}_sql
    NAMESPACE
    sql_queries
    OUTPUT_DIR
    ${CMAKE_CURRENT_BINARY_DIR}
    SQL_FILES
    src/queries/**/*.sql
)

add_library(${PROJECT_NAME}_objs OBJECT
    # src/components
    src/components/component_list.cpp
    src/components/storage/storage.cpp
    src/components/hello_grpc/hello_grpc.cpp
    src/components/game_service/game_service_component.cpp

    # src/handlers
    src/handlers/component_list.cpp

    ## src/handlers/content_handling
    src/handlers/content_handling/component_list.cpp
    src/handlers/content_handling/pack/component_list.cpp
    src/handlers/content_handling/pack/create_pack.cpp
    src/handlers/content_handling/pack/create_pack_from_yaml.cpp
    src/handlers/content_handling/pack/get_all_packs.cpp
    src/handlers/content_handling/pack/get_pack_by_id.cpp
    src/handlers/content_handling/question/component_list.cpp
    src/handlers/content_handling/question/create_question.cpp
    src/handlers/content_handling/question/get_question_by_id.cpp
    src/handlers/content_handling/question/get_questions_by_pack_id.cpp
    src/handlers/content_handling/variant/component_list.cpp
    src/handlers/content_handling/variant/create_variant.cpp
    src/handlers/content_handling/variant/get_variant_by_id.cpp
    src/handlers/content_handling/variant/get_variants_by_question_id.cpp

    ## src/handlers/grpc
    src/handlers/grpc/component_list.cpp
    src/handlers/grpc/service.cpp

    ## src/handlers/hello
    src/handlers/hello/component_list.cpp
    src/handlers/hello/hello.cpp

    ## src/handlers/hello_postgres
    src/handlers/hello_postgres/component_list.cpp
    src/handlers/hello_postgres/hello_postgres.cpp

    ## src/handlers/game
    src/handlers/game/component_list.cpp
    src/handlers/game/create_game_session.cpp
    src/handlers/game/add_player.cpp
    src/handlers/game/start_game.cpp
    src/handlers/game/get_game_state.cpp
    src/handlers/game/submit_answer.cpp
    src/handlers/game/get_game_results.cpp

    # src/logic
    src/logic/game/game.cpp
    src/logic/greeting/greeting.cpp
    src/logic/validators/answer_validator.cpp
    src/logic/validators/validator_factory.cpp

    # src/models
    src/models/game_session.cpp
    src/models/pack.cpp
    src/models/player_answer.cpp
    src/models/player.cpp
    src/models/question.cpp
    src/models/question_type.cpp
    src/models/text_answer.cpp
    src/models/variant.cpp

    src/storage/game_sessions.cpp
    src/storage/packs.cpp
    src/storage/player_answers.cpp
    src/storage/players.cpp
    src/storage/questions_and_variants.cpp
    src/storage/questions.cpp
    src/storage/text_answers.cpp
    src/storage/variants.cpp

    # src/utils
    src/utils/pack.cpp
    src/utils/question.cpp
    src/utils/string_to_uuid.cpp
    src/utils/variant.cpp
)
target_link_libraries(${PROJECT_NAME}_objs PUBLIC
  userver::core
  userver::grpc
  userver::postgresql
  ${PROJECT_NAME}_sql
)


# Create a proto library with userver extensions
userver_add_grpc_library(
    ${PROJECT_NAME}_proto
    PROTOS
    models/models.proto

    handlers/hello.proto
    handlers/cruds.proto
)
target_link_libraries(${PROJECT_NAME}_objs PUBLIC ${PROJECT_NAME}_proto)


# The Service
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_objs)


# Unit Tests
add_executable(${PROJECT_NAME}_unittest
    tests/unit/string_to_bool_test.cpp
    tests/unit/string_to_uuid_test.cpp
    tests/unit/greeting_test.cpp
)
target_link_libraries(${PROJECT_NAME}_unittest PRIVATE ${PROJECT_NAME}_objs userver::utest)
add_google_tests(${PROJECT_NAME}_unittest)


# # Benchmarks
# add_executable(${PROJECT_NAME}_benchmark
#     src/benchmarks/greeting_benchmark.cpp
# )
# target_link_libraries(${PROJECT_NAME}_benchmark PRIVATE ${PROJECT_NAME}_objs userver::ubench)
# add_google_benchmark_tests(${PROJECT_NAME}_benchmark)

# Functional testing
userver_testsuite_add_simple()

# Install
include(GNUInstallDirs)

if(DEFINED ENV{PREFIX})
  message(STATUS "Set install prefix: $ENV{PREFIX}")
  file(TO_CMAKE_PATH "$ENV{PREFIX}" PREFIX_PATH)
  set(CMAKE_INSTALL_PREFIX "${PREFIX_PATH}")
endif()

file(GLOB CONFIGS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.yaml ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.json)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT ${PROJECT_NAME})
install(FILES ${CONFIGS_FILES} DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME} COMPONENT ${PROJECT_NAME})
