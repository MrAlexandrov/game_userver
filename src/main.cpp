#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/components/component.hpp>
#include <userver/components/component_list.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/congestion_control/component.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/testsuite/testsuite_support.hpp>

#include <userver/storages/postgres/component.hpp>
#include <userver/ugrpc/server/component_list.hpp>

#include <userver/utils/daemon_run.hpp>

#include "handlers/content_handling/pack/create_pack.hpp"
#include "handlers/content_handling/pack/get_all_packs.hpp"
#include "handlers/content_handling/pack/get_pack_by_id.hpp"
#include "handlers/content_handling/question/create_question.hpp"
#include "handlers/content_handling/question/get_question_by_id.hpp"
#include "handlers/content_handling/question/get_questions_by_pack_id.hpp"
#include "handlers/content_handling/variant/create_variant.hpp"
#include "handlers/content_handling/variant/get_variant_by_id.hpp"
#include "handlers/content_handling/variant/get_variants_by_question_id.hpp"

#include "handlers/grpc/pack/create_pack.hpp"
#include "handlers/hello/hello.hpp"
#include "components/hello_grpc/hello_grpc.hpp"
#include "handlers/hello_postgres/hello_postgres.hpp"

int main(int argc, char* argv[]) {
    auto component_list = userver::components::MinimalServerComponentList()
                              .Append<userver::server::handlers::Ping>()
                              .Append<userver::components::TestsuiteSupport>()
                              .Append<userver::components::HttpClient>()
                              .Append<userver::clients::dns::Component>()
                              .Append<userver::server::handlers::TestsControl>()
                              .Append<userver::congestion_control::Component>()
                              .Append<game_userver::Hello>()
                              .Append<userver::components::Postgres>("postgres-db-1")
                              .Append<game_userver::HelloPostgres>()
                              .AppendComponentList(userver::ugrpc::server::MinimalComponentList())
                              .Append<game_userver::HelloGrpc>()
                              .Append<game_userver::CreatePack>()
                              .Append<game_userver::GetAllPacks>()
                              .Append<game_userver::GetPack>()
                              .Append<game_userver::CreateQuestion>()
                              .Append<game_userver::GetQuestionById>()
                              .Append<game_userver::GetQuestionsByPackId>()
                              .Append<game_userver::CreateVariant>()
                              .Append<game_userver::GetVariantById>()
                              .Append<game_userver::GetVariantsByQuestionId>()
                              .Append<game_userver::CreatePackGrpc>()
        ;

    return userver::utils::DaemonMain(argc, argv, component_list);
}
