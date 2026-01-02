#include "variant_service.hpp"

#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

#include "storage/variants.hpp"
#include "utils/constants.hpp"

namespace game_userver::services {

VariantService::VariantService(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : LoggableComponentBase(config, context),
      pg_cluster_(context
                      .FindComponent<userver::components::Postgres>(
                          Constants::kDatabaseName
                      )
                      .GetCluster()) {}

VariantService::~VariantService() = default;

std::optional<Models::Variant> VariantService::CreateVariant(
    const boost::uuids::uuid& question_id, const std::string& text,
    bool is_correct
) {
    return NStorage::CreateVariant(pg_cluster_, question_id, text, is_correct);
}

std::optional<Models::Variant>
VariantService::GetVariantById(const boost::uuids::uuid& variant_id) {
    return NStorage::GetVariantById(pg_cluster_, variant_id);
}

std::vector<Models::Variant>
VariantService::GetVariantsByQuestionId(const boost::uuids::uuid& question_id) {
    return NStorage::GetVariantsByQuestionId(pg_cluster_, question_id);
}

} // namespace game_userver::services
