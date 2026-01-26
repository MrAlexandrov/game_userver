#include "game_service_component.hpp"

#include <userver/components/component_context.hpp>
#include <userver/logging/log.hpp>
#include <userver/yaml_config/schema.hpp>

#include "utils/constants.hpp"

namespace game_userver::components {

GameServiceComponent::GameServiceComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : ComponentBase(config, context) {
    // Получаем PostgreSQL cluster из контекста
    auto& pg_component = context.FindComponent<userver::components::Postgres>(
        Constants::kDatabaseName
    );
    auto pg_cluster = pg_component.GetCluster();

    // Создаем единственный экземпляр GameService
    game_service_ = std::make_unique<logic::game::GameService>(pg_cluster);

    LOG_INFO() << "GameServiceComponent initialized successfully";
}

GameServiceComponent::~GameServiceComponent() {
    LOG_INFO() << "GameServiceComponent destroyed";
}

logic::game::GameService& GameServiceComponent::GetGameService() {
    return *game_service_;
}

const logic::game::GameService& GameServiceComponent::GetGameService() const {
    return *game_service_;
}

userver::yaml_config::Schema GameServiceComponent::GetStaticConfigSchema() {
    return userver::yaml_config::MakeSchema<
        userver::components::ComponentConfig>();
}

} // namespace game_userver::components
