#include "players.hpp"

#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/cluster_types.hpp>

#include <samples_postgres_service/sql_queries.hpp>

#include <userver/storages/postgres/io/io_fwd.hpp>

namespace NStorage {

using userver::storages::postgres::ClusterPtr;
using namespace samples_postgres_service::sql;
using userver::storages::postgres::ClusterHostType::kMaster;
using userver::storages::postgres::ClusterHostType::kSlave;

auto AddPlayer(ClusterPtr pg_cluster_, const boost::uuids::uuid& game_session_id, const std::string& name) -> std::optional<NModels::Player> {
    auto result = pg_cluster_->Execute(
        kMaster,
        kAddPlayer,
        game_session_id,
        name
    );
    return result.AsOptionalSingleRow<NModels::Player>(userver::storages::postgres::kRowTag);
}

auto GetPlayersByGameSessionId(ClusterPtr pg_cluster_, const boost::uuids::uuid& game_session_id) -> std::vector<NModels::Player> {
    auto result = pg_cluster_->Execute(
        kSlave,
        kGetPlayersByGameSessionId,
        game_session_id
    );
    return result.AsContainer<std::vector<NModels::Player>>(userver::storages::postgres::kRowTag);
}

auto UpdatePlayerScore(ClusterPtr pg_cluster_, const boost::uuids::uuid& player_id, int points) -> std::optional<NModels::Player> {
    auto result = pg_cluster_->Execute(
        kMaster,
        kUpdatePlayerScore,
        player_id,
        points
    );
    return result.AsOptionalSingleRow<NModels::Player>(userver::storages::postgres::kRowTag);
}

} // namespace NStorage