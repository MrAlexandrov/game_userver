#include "component_list.hpp"

#include "create_pack.hpp"
#include "get_all_packs.hpp"
#include "get_pack_by_id.hpp"

namespace game_userver::pack {

auto GetPackHandlersComponentList() -> userver::components::ComponentList {
    return userver::components::ComponentList()
        .Append<CreatePack>()
        .Append<GetAllPacks>()
        .Append<GetPack>();
}

} // namespace game_userver::pack
