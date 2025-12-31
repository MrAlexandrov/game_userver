#include "component_list.hpp"

#include "hello_postgres.hpp"

namespace game_userver {

userver::components::ComponentList GetHelloPostgresComponentList() {
    return userver::components::ComponentList()
        .Append<game_userver::HelloPostgres>();
}

} // namespace game_userver
