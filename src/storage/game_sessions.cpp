#include "game_sessions.hpp"

#include <sql_queries/sql_queries.hpp>
#include <userver/storages/postgres/cluster_types.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/io/io_fwd.hpp>

#include "models/game_session.hpp"

namespace NStorage {

using userver::storages::postgres::ClusterPtr;
using namespace sql_queries::sql;
using userver::storages::postgres::ClusterHostType::kMaster;
using userver::storages::postgres::ClusterHostType::kSlave;

auto CreateGameSession(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& pack_id
) -> std::optional<Models::GameSession> {
    auto result = pg_cluster_->Execute(kMaster, kCreateGameSession, pack_id);
    return result.AsOptionalSingleRow<Models::GameSession>(
        userver::storages::postgres::kRowTag
    );
}

auto GetGameSessionById(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& game_session_id
) -> std::optional<Models::GameSession> {
    auto result =
        pg_cluster_->Execute(kMaster, kGetGameSessionById, game_session_id);
    return result.AsOptionalSingleRow<Models::GameSession>(
        userver::storages::postgres::kRowTag
    );
}

auto StartGameSession(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& game_session_id
) -> std::optional<Models::GameSession> {
    auto result =
        pg_cluster_->Execute(kMaster, kStartGameSession, game_session_id);
    return result.AsOptionalSingleRow<Models::GameSession>(
        userver::storages::postgres::kRowTag
    );
}

auto AdvanceToNextQuestion(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& game_session_id,
    int current_question_index
) -> std::optional<Models::GameSession> {
    auto result = pg_cluster_->Execute(
        kMaster, kUpdateGameSessionCurrentQuestionIndex, game_session_id,
        current_question_index
    );
    return result.AsOptionalSingleRow<Models::GameSession>(
        userver::storages::postgres::kRowTag
    );
}

auto EndGameSession(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& game_session_id
) -> std::optional<Models::GameSession> {
    auto result =
        pg_cluster_->Execute(kMaster, kEndGameSession, game_session_id);
    return result.AsOptionalSingleRow<Models::GameSession>(
        userver::storages::postgres::kRowTag
    );
}

auto GetAllGameSessions(ClusterPtr pg_cluster_)
    -> std::vector<Models::GameSession> {
    auto result = pg_cluster_->Execute(kSlave, kGetAllGameSessions);
    return result.AsContainer<std::vector<Models::GameSession>>(
        userver::storages::postgres::kRowTag
    );
}

} // namespace NStorage
