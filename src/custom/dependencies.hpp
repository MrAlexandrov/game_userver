#pragma once

#include <userver/components/component_fwd.hpp>
#include <userver/storages/secdist/fwd.hpp>

namespace Custom {

class Dependencies {
public:
    Dependencies(
        const userver::components::ComponentConfig&,
        const userver::components::ComponentContext&
    );

    struct Extra {};

    [[nodiscard]] auto GetExtra() const -> Extra;

private:
};

} // namespace Custom
