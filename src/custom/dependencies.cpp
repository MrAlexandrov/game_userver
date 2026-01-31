#include "dependencies.hpp"

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/secdist/component.hpp>

namespace Custom {

Dependencies::Dependencies(
    [[maybe_unused]] const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& ctx
)
    {}

auto Dependencies::GetExtra() const -> Dependencies::Extra {
    return {};
}

};  // namespace Custom
