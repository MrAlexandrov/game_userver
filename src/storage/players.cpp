#include "players.hpp"

#include <sql_queries/sql_queries.hpp>
#include <userver/storages/postgres/cluster_types.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/io/io_fwd.hpp>

#include "models/player.hpp"

namespace NStorage {

using userver::storages::postgres::ClusterPtr;
using namespace sql_queries::sql;
using userver::storages::postgres::ClusterHostType::kMaster;
using userver::storages::postgres::ClusterHostType::kSlave;

auto AddPlayer(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& game_session_id,
    const std::string& name
) -> std::optional<Models::Player> {
    auto result =
        pg_cluster_->Execute(kMaster, kAddPlayer, game_session_id, name);
    return result.AsOptionalSingleRow<Models::Player>(
        userver::storages::postgres::kRowTag
    );
}

auto GetPlayerById(ClusterPtr pg_cluster_, const boost::uuids::uuid& player_id)
    -> std::optional<Models::Player> {
    auto result = pg_cluster_->Execute(kMaster, kGetPlayerById, player_id);
    return result.AsOptionalSingleRow<Models::Player>(
        userver::storages::postgres::kRowTag
    );
}

auto GetPlayersByGameSessionId(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& game_session_id
) -> std::vector<Models::Player> {
    auto result = pg_cluster_->Execute(
        kSlave, kGetPlayersByGameSessionId, game_session_id
    );
    return result.AsContainer<std::vector<Models::Player>>(
        userver::storages::postgres::kRowTag
    );
}

auto UpdatePlayerScore(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& player_id,
    int score_increment
) -> std::optional<Models::Player> {
    auto result = pg_cluster_->Execute(
        kMaster, kUpdatePlayerScore, player_id, score_increment
    );
    return result.AsOptionalSingleRow<Models::Player>(
        userver::storages::postgres::kRowTag
    );
}

} // namespace NStorage
