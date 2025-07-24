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

#include "handlers/content_handling/pack/create.hpp"
#include "handlers/content_handling/pack/get.hpp"

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
                              .Append<game_userver::GetPack>()
        ;

    return userver::utils::DaemonMain(argc, argv, component_list);
}
