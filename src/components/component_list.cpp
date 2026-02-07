#include "component_list.hpp"

#include "game_service/game_service_component.hpp"
#include "hello_grpc/hello_grpc.hpp"
#include "storage/storage.hpp"

namespace game_userver {

auto GetComponentsComponentList() -> userver::components::ComponentList {
    return userver::components::ComponentList()
        .Append<game_userver::components::GameServiceComponent>()
        .Append<HelloGrpc>()
        .Append<game_userver::components::Storage>();
}

} // namespace game_userver
