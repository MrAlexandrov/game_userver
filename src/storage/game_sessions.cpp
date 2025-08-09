#include "game_sessions.hpp"

#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/cluster_types.hpp>

#include <samples_postgres_service/sql_queries.hpp>

#include <userver/storages/postgres/io/io_fwd.hpp>

namespace NStorage {

using userver::storages::postgres::ClusterPtr;
using namespace samples_postgres_service::sql;
using userver::storages::postgres::ClusterHostType::kMaster;
using userver::storages::postgres::ClusterHostType::kSlave;

auto CreateGameSession(ClusterPtr pg_cluster_, const boost::uuids::uuid& pack_id) -> std::optional<NModels::GameSession> {
    auto result = pg_cluster_->Execute(
        kMaster,
        kCreateGameSession,
        pack_id
    );
    return result.AsOptionalSingleRow<NModels::GameSession>(userver::storages::postgres::kRowTag);
}

auto GetGameSessionById(ClusterPtr pg_cluster_, const boost::uuids::uuid& game_session_id) -> std::optional<NModels::GameSession> {
    auto result = pg_cluster_->Execute(
        kMaster,
        kGetGameSessionById,
        game_session_id
    );
    return result.AsOptionalSingleRow<NModels::GameSession>(userver::storages::postgres::kRowTag);
}

auto StartGameSession(ClusterPtr pg_cluster_, const boost::uuids::uuid& game_session_id) -> std::optional<NModels::GameSession> {
    auto result = pg_cluster_->Execute(
        kMaster,
        kStartGameSession,
        game_session_id
    );
    return result.AsOptionalSingleRow<NModels::GameSession>(userver::storages::postgres::kRowTag);
}

auto EndGameSession(ClusterPtr pg_cluster_, const boost::uuids::uuid& game_session_id) -> std::optional<NModels::GameSession> {
    auto result = pg_cluster_->Execute(
        kMaster,
        kEndGameSession,
        game_session_id
    );
    return result.AsOptionalSingleRow<NModels::GameSession>(userver::storages::postgres::kRowTag);
}

auto GetAllGameSessions(ClusterPtr pg_cluster_) -> std::vector<NModels::GameSession> {
    auto result = pg_cluster_->Execute(
        kSlave,
        kGetAllGameSessions
    );
    return result.AsContainer<std::vector<NModels::GameSession>>(userver::storages::postgres::kRowTag);
}

} // namespace NStorage