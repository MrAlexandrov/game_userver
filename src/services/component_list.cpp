#include "component_list.hpp"

#include "pack_service.hpp"
#include "question_service.hpp"
#include "variant_service.hpp"

namespace game_userver::services {

userver::components::ComponentList GetServicesComponentList() {
    return userver::components::ComponentList()
        .Append<PackService>()
        .Append<QuestionService>()
        .Append<VariantService>();
}

} // namespace game_userver::services
