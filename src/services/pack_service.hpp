#pragma once

#include <optional>
#include <string>
#include <vector>

#include <boost/uuid/uuid.hpp>
#include <userver/components/loggable_component_base.hpp>
#include <userver/storages/postgres/postgres_fwd.hpp>

#include "models/pack.hpp"

namespace game_userver::services {

class PackService final : public userver::components::LoggableComponentBase {
public:
    static constexpr std::string_view kName = "pack-service";

    PackService(
        const userver::components::ComponentConfig&,
        const userver::components::ComponentContext&
    );

    ~PackService() override;

    // Business logic methods
    std::optional<Models::Pack> CreatePack(const std::string& title);
    std::optional<Models::Pack> GetPackById(const boost::uuids::uuid& pack_id);
    std::vector<Models::Pack> GetAllPacks();

private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};

} // namespace game_userver::services
