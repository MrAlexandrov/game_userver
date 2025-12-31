#include "component_list.hpp"

#include "hello.hpp"

namespace game_userver {

userver::components::ComponentList GetHelloComponentList() {
    return userver::components::ComponentList().Append<game_userver::Hello>();
}

} // namespace game_userver
