#pragma once

#include <userver/components/component.hpp>

#include <handlers/cruds_service.usrv.pb.hpp>
#include <handlers/cruds.pb.h> // for responce
#include <models/pack.hpp> // for responce
#include <userver/storages/postgres/postgres_fwd.hpp>

#include <models/game_session.hpp>
#include <models/player.hpp>
#include <models/player_answer.hpp>

namespace game_userver {

class Service final : public handlers::api::QuizServiceBase::Component {
public:
    static constexpr std::string_view kName = "handler-service-grpc";

    Service(
          const userver::components::ComponentConfig&
        , const userver::components::ComponentContext&
    );

    CreatePackResult CreatePack(
          CallContext&
        , handlers::api::CreatePackRequest&&
    ) override;

    GetPackByIdResult GetPackById(
          CallContext&
        , handlers::api::GetPackByIdRequest&&
    ) override;

    GetAllPacksResult GetAllPacks(
          CallContext&
        , handlers::api::GetAllPacksRequest&&
    ) override;

    CreateQuestionResult CreateQuestion(
        CallContext&,
        handlers::api::CreateQuestionRequest&&
    ) override;

    GetQuestionByIdResult GetQuestionById(
        CallContext&,
        handlers::api::GetQuestionByIdRequest&&
    ) override;

    GetQuestionsByPackIdResult GetQuestionsByPackId(
        CallContext&,
        handlers::api::GetQuestionsByPackIdRequest&&
    ) override;

    // === Variant operations ===
    CreateVariantResult CreateVariant(
        CallContext&,
        handlers::api::CreateVariantRequest&&
    ) override;

    GetVariantByIdResult GetVariantById(
        CallContext&,
        handlers::api::GetVariantByIdRequest&&
    ) override;
    GetVariantsByQuestionIdResult GetVariantsByQuestionId(
        CallContext&,
        handlers::api::GetVariantsByQuestionIdRequest&&
    ) override;

    // === Game Session operations ===
    CreateGameSessionResult CreateGameSession(
          CallContext&
        , handlers::api::CreateGameSessionRequest&&
    ) override;

    GetGameSessionResult GetGameSession(
          CallContext&
        , handlers::api::GetGameSessionRequest&&
    ) override;

    StartGameSessionResult StartGameSession(
          CallContext&
        , handlers::api::StartGameSessionRequest&&
    ) override;

    EndGameSessionResult EndGameSession(
          CallContext&
        , handlers::api::EndGameSessionRequest&&
    ) override;

    GetGameSessionsResult GetGameSessions(
          CallContext&
        , handlers::api::GetGameSessionsRequest&&
    ) override;

    // === Player operations ===
    AddPlayerResult AddPlayer(
        CallContext&,
        handlers::api::AddPlayerRequest&&
    ) override;

    GetPlayersResult GetPlayers(
        CallContext&,
        handlers::api::GetPlayersRequest&&
    ) override;

    // === Player Answer operations ===
    SubmitAnswerResult SubmitAnswer(
        CallContext&,
        handlers::api::SubmitAnswerRequest&&
    ) override;

    GetPlayerAnswersResult GetPlayerAnswers(
        CallContext&,
        handlers::api::GetPlayerAnswersRequest&&
    ) override;

    // === Additional game flow operations ===
    GetCurrentQuestionResult GetCurrentQuestion(
        CallContext&,
        handlers::api::GetCurrentQuestionRequest&&
    ) override;

    AdvanceToNextQuestionResult AdvanceToNextQuestion(
        CallContext&,
        handlers::api::AdvanceToNextQuestionRequest&&
    ) override;

    GetQuestionsWithVariantsResult GetQuestionsWithVariants(
        CallContext&,
        handlers::api::GetQuestionsWithVariantsRequest&&
    ) override;

private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};

} // namespace game_userver
