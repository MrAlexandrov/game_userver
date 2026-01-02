#include "component_list.hpp"

#include "pack/component_list.hpp"
#include "question/component_list.hpp"
#include "variant/component_list.hpp"

namespace game_userver {

auto GetContentHandlingComponentList() -> userver::components::ComponentList {
    return userver::components::ComponentList()
        .AppendComponentList(pack::GetPackHandlersComponentList())
        .AppendComponentList(question::GetQuestionHandlersComponentList())
        .AppendComponentList(variant::GetVariantHandlersComponentList());
}

} // namespace game_userver
