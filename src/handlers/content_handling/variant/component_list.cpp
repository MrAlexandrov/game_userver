#include "component_list.hpp"

#include "create_variant.hpp"
#include "get_variant_by_id.hpp"
#include "get_variants_by_question_id.hpp"

namespace game_userver::variant {

userver::components::ComponentList GetVariantHandlersComponentList() {
    return userver::components::ComponentList()
        .Append<CreateVariant>()
        .Append<GetVariantById>()
        .Append<GetVariantsByQuestionId>();
}

} // namespace game_userver::variant