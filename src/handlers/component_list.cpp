#include "component_list.hpp"

#include "content_handling/component_list.hpp"
#include "grpc/component_list.hpp"
#include "hello/component_list.hpp"
#include "hello_postgres/component_list.hpp"

namespace game_userver {

userver::components::ComponentList GetHandlersComponentList() {
    return userver::components::ComponentList()
        .AppendComponentList(game_userver::GetContentHandlingComponentList())
        .AppendComponentList(game_userver::GetGrpcComponentList())
        .AppendComponentList(game_userver::GetHelloComponentList())
        .AppendComponentList(game_userver::GetHelloPostgresComponentList());
}

} // namespace game_userver
