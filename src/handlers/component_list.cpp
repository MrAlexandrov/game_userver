#include "component_list.hpp"

#include "content_handling/component_list.hpp"
#include "game/component_list.hpp"
#include "grpc/component_list.hpp"
#include "hello/component_list.hpp"
#include "hello_postgres/component_list.hpp"

namespace game_userver {

auto GetHandlersComponentList() -> userver::components::ComponentList {
    auto component_list =
        userver::components::ComponentList()
            .AppendComponentList(
                game_userver::GetContentHandlingComponentList()
            )
            .AppendComponentList(game_userver::GetGrpcComponentList())
            .AppendComponentList(game_userver::GetHelloComponentList())
            .AppendComponentList(game_userver::GetHelloPostgresComponentList());

    game_userver::AppendGameHandlers(component_list);

    return component_list;
}

} // namespace game_userver
