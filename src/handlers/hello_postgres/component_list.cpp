#include "component_list.hpp"

#include "hello_postgres.hpp"

namespace game_userver {

auto GetHelloPostgresComponentList() -> userver::components::ComponentList {
    return userver::components::ComponentList()
        .Append<game_userver::HelloPostgres>();
}

} // namespace game_userver
