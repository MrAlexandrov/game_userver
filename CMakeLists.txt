cmake_minimum_required(VERSION 3.12...3.31)
project(game_userver CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(DownloadUserver)

find_package(userver COMPONENTS
    core
    grpc
    postgresql
    QUIET
)
if(NOT userver_FOUND)
  # Tries TRY_DIR first, falls back to downloading userver from GitHub using CPM.
  download_userver(TRY_DIR third_party/userver)
endif()

userver_setup_environment()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Common sources
include_directories(src)

userver_add_sql_library(
    ${PROJECT_NAME}_sql
    NAMESPACE
    samples_postgres_service
    OUTPUT_DIR
    ${CMAKE_CURRENT_BINARY_DIR}
    SQL_FILES
    src/queries/*.sql
)

add_library(${PROJECT_NAME}_objs OBJECT
    src/components/hello_grpc/hello_grpc.cpp

    src/handlers/grpc/pack/create_pack.cpp
    src/handlers/grpc/pack/responce.cpp

    src/handlers/hello/hello.cpp
    src/handlers/hello_postgres/hello_postgres.cpp

    src/handlers/content_handling/pack/create_pack.cpp
    src/handlers/content_handling/pack/get_all_packs.cpp
    src/handlers/content_handling/pack/get_pack_by_id.cpp
    src/handlers/content_handling/question/create_question.cpp
    src/handlers/content_handling/question/get_question_by_id.cpp
    src/handlers/content_handling/question/get_questions_by_pack_id.cpp
    src/handlers/content_handling/variant/create_variant.cpp
    src/handlers/content_handling/variant/get_variant_by_id.cpp
    src/handlers/content_handling/variant/get_variants_by_question_id.cpp

    src/logic/greeting/greeting.cpp

    src/models/pack.cpp
    src/models/question.cpp
    src/models/variant.cpp

    src/storage/pasks.cpp
    src/storage/questions.cpp
    src/storage/variants.cpp

    src/utils/string_to_uuid.cpp
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
