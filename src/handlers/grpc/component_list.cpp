#include "component_list.hpp"

#include "service.hpp"

namespace game_userver {

userver::components::ComponentList GetGrpcComponentList() {
    return userver::components::ComponentList()
        .Append<game_userver::Service>();
}

} // namespace game_userver
