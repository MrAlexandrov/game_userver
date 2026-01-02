#include "component_list.hpp"

#include "service.hpp"

namespace game_userver {

auto GetGrpcComponentList() -> userver::components::ComponentList {
    return userver::components::ComponentList().Append<game_userver::Service>();
}

} // namespace game_userver
