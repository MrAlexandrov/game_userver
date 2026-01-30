#pragma once

#include <memory>
#include <string_view>

#include <userver/components/component.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/yaml_config/schema.hpp>

#include "logic/game/game.hpp"

namespace game_userver::components {

// Singleton component для GameService
// Обеспечивает единственный экземпляр GameService на весь сервер
// и сохраняет observers между запросами
class GameServiceComponent final : public userver::components::ComponentBase {
public:
    static constexpr std::string_view kName = "game-service";

    GameServiceComponent(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    );

    ~GameServiceComponent() override;

    // Получить ссылку на GameService
    [[nodiscard]] logic::game::GameService& GetGameService();
    [[nodiscard]] const logic::game::GameService& GetGameService() const;

    // static userver::yaml_config::Schema GetStaticConfigSchema();

private:
    std::unique_ptr<logic::game::GameService> game_service_;
};

} // namespace game_userver::components
