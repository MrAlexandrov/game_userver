#pragma once

#include <optional>
#include <string>
#include <vector>

#include <boost/uuid/uuid.hpp>
#include <userver/components/loggable_component_base.hpp>
#include <userver/storages/postgres/postgres_fwd.hpp>

#include "models/variant.hpp"

namespace game_userver::services {

class VariantService final : public userver::components::LoggableComponentBase {
public:
    static constexpr std::string_view kName = "variant-service";

    VariantService(
        const userver::components::ComponentConfig&,
        const userver::components::ComponentContext&
    );

    ~VariantService() override;

    // Business logic methods
    std::optional<Models::Variant> CreateVariant(
        const boost::uuids::uuid& question_id, const std::string& text,
        bool is_correct
    );

    std::optional<Models::Variant>
    GetVariantById(const boost::uuids::uuid& variant_id);

    std::vector<Models::Variant>
    GetVariantsByQuestionId(const boost::uuids::uuid& question_id);

private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};

} // namespace game_userver::services
