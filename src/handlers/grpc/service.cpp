#include "service.hpp"

#include "storage/packs.hpp" // for db request CreatePack
#include "storage/questions.hpp"
#include "storage/variants.hpp"
#include "storage/game_sessions.hpp"
#include "storage/players.hpp"
#include "storage/player_answers.hpp"

#include <boost/uuid/uuid_io.hpp> // for responce
#include <models/models.pb.h> // proto model Pack
#include <models/pack.hpp> // cpp model Pack
#include <models/question.hpp>
#include <models/variant.hpp>
#include <models/game_session.hpp>
#include <models/player.hpp>
#include <models/player_answer.hpp>
#include <userver/storages/postgres/component.hpp>
#include <utils/string_to_uuid.hpp>

namespace game_userver {

Service::Service(
      const userver::components::ComponentConfig& config
    , const userver::components::ComponentContext& component_context
)
    : handlers::api::QuizServiceBase::Component(config, component_context)
    , pg_cluster_(
        component_context.FindComponent<userver::components::Postgres>("postgres-db-1")
            .GetCluster()
    )
{
}

Service::CreatePackResult Service::CreatePack(
      CallContext&
    , handlers::api::CreatePackRequest&& request
) {
    if (request.title().empty()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Title cannot be empty"
        };
    }
    
    auto createdPackOpt = NStorage::CreatePack(pg_cluster_, request.title());

    if (!createdPackOpt.has_value()) {
        return grpc::Status{
            grpc::StatusCode::INTERNAL,
            "Failed to create pack"
        };
    }
    auto createdPack = createdPackOpt.value();

    handlers::api::CreatePackResponse responce;
    auto mutualPack = responce.mutable_pack();
    mutualPack->set_id(boost::uuids::to_string(std::move(createdPack.id)));
    mutualPack->set_title(std::move(createdPack.title));
    return responce;
}

Service::GetPackByIdResult Service::GetPackById(
      CallContext&
    , handlers::api::GetPackByIdRequest&& request
) {
    auto pack_id = NUtils::StringToUuid(request.id());
    if (pack_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.id()
        };
    }
    auto getPackByIdOpt = NStorage::GetPackById(pg_cluster_, pack_id);

    if (!getPackByIdOpt.has_value()) {
        return grpc::Status{
            grpc::StatusCode::NOT_FOUND,
            "Pack not found"
        };
    }

    auto getPackById = getPackByIdOpt.value();

    handlers::api::GetPackByIdResponse responce;
    auto mutualPack = responce.mutable_pack();
    mutualPack->set_id(boost::uuids::to_string(std::move(getPackById.id)));
    mutualPack->set_title(std::move(getPackById.title));
    return responce;
}

Service::GetAllPacksResult Service::GetAllPacks(
      CallContext&
    , handlers::api::GetAllPacksRequest&& request
) {
    auto getAllPacks = NStorage::GetAllPacks(pg_cluster_);

    handlers::api::GetAllPacksResponse responce;
    auto mutualPacks = responce.mutable_packs();
    for (auto&& pack : getAllPacks) {
        NModels::Proto::Pack packResponse;
        packResponse.set_id(boost::uuids::to_string(pack.id));
        packResponse.set_title(std::move(pack.title));

        mutualPacks->Add(std::move(packResponse));
    }
    return responce;
}

Service::CreateQuestionResult Service::CreateQuestion(
      CallContext&
    , handlers::api::CreateQuestionRequest&& request
) {
    if (request.text().empty()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Question text cannot be empty"
        };
    }
    
    auto pack_id = NUtils::StringToUuid(request.pack_id());
    if (pack_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.pack_id()
        };
    }
    auto createdQuestionOpt = NStorage::CreateQuestion(
        pg_cluster_,
        pack_id,
        request.text(),
        request.image_url()
    );

    if (!createdQuestionOpt.has_value()) {
        return grpc::Status{
            grpc::StatusCode::INTERNAL,
            "Failed to create question"
        };
    }
    auto createdQuestion = createdQuestionOpt.value();

    handlers::api::CreateQuestionResponse response;
    auto mutableQuestion = response.mutable_question();
    mutableQuestion->set_id(boost::uuids::to_string(std::move(createdQuestion.id)));
    mutableQuestion->set_pack_id(boost::uuids::to_string(std::move(createdQuestion.pack_id)));
    mutableQuestion->set_text(std::move(createdQuestion.text));
    if (!createdQuestion.image_url.empty()) {
        mutableQuestion->set_image_url(createdQuestion.image_url);
    }

    return response;
}

Service::GetQuestionByIdResult Service::GetQuestionById(
      CallContext&
    , handlers::api::GetQuestionByIdRequest&& request
) {
    auto question_id = NUtils::StringToUuid(request.id());
    if (question_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.id()
        };
    }
    auto questionOpt = NStorage::GetQuestionById(
        pg_cluster_,
        question_id
    );

    if (!questionOpt.has_value()) {
        return grpc::Status{
            grpc::StatusCode::NOT_FOUND,
            "Question not found"
        };
    }
    auto question = questionOpt.value();

    handlers::api::GetQuestionByIdResponse response;
    auto mutableQuestion = response.mutable_question();
    mutableQuestion->set_id(boost::uuids::to_string(std::move(question.id)));
    mutableQuestion->set_pack_id(boost::uuids::to_string(std::move(question.pack_id)));
    mutableQuestion->set_text(std::move(question.text));
    if (!question.image_url.empty()) {
        mutableQuestion->set_image_url(std::move(question.image_url));
    }

    return response;
}

Service::GetQuestionsByPackIdResult Service::GetQuestionsByPackId(
      CallContext&
    , handlers::api::GetQuestionsByPackIdRequest&& request
) {
    auto pack_id = NUtils::StringToUuid(request.pack_id());
    if (pack_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.pack_id()
        };
    }
    auto questions = NStorage::GetQuestionsByPackId(
        pg_cluster_,
        pack_id
    );

    handlers::api::GetQuestionsByPackIdResponse response;
    auto mutableQuestions = response.mutable_questions();

    for (auto&& question : questions) {
        auto newQuestion = mutableQuestions->Add();
        newQuestion->set_id(boost::uuids::to_string(question.id));
        newQuestion->set_pack_id(boost::uuids::to_string(question.pack_id));
        newQuestion->set_text(std::move(question.text));
        if (!question.image_url.empty()) {
            newQuestion->set_image_url(std::move(question.image_url));
        }
    }

    return response;
}

Service::CreateVariantResult Service::CreateVariant(
      CallContext&
    , handlers::api::CreateVariantRequest&& request
) {
    if (request.text().empty()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Variant text cannot be empty"
        };
    }
    
    auto question_id = NUtils::StringToUuid(request.question_id());
    if (question_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.question_id()
        };
    }
    auto createdVariantOpt = NStorage::CreateVariant(
        pg_cluster_,
        question_id,
        request.text(),
        request.is_correct()
    );

    if (!createdVariantOpt.has_value()) {
        return grpc::Status{
            grpc::StatusCode::INTERNAL,
            "Failed to create variant"
        };
    }
    auto createdVariant = createdVariantOpt.value();

    handlers::api::CreateVariantResponse response;
    auto mutableVariant = response.mutable_variant();
    mutableVariant->set_id(boost::uuids::to_string(std::move(createdVariant.id)));
    mutableVariant->set_question_id(boost::uuids::to_string(std::move(createdVariant.question_id)));
    mutableVariant->set_text(std::move(createdVariant.text));
    mutableVariant->set_is_correct(createdVariant.is_correct);

    return response;
}

Service::GetVariantByIdResult Service::GetVariantById(
      CallContext&
    , handlers::api::GetVariantByIdRequest&& request
) {
    auto variant_id = NUtils::StringToUuid(request.id());
    if (variant_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.id()
        };
    }
    auto variantOpt = NStorage::GetVariantById(
        pg_cluster_,
        variant_id
    );

    if (!variantOpt.has_value()) {
        return grpc::Status{
            grpc::StatusCode::NOT_FOUND,
            "Variant not found"
        };
    }
    auto variant = variantOpt.value();

    handlers::api::GetVariantByIdResponse response;
    auto mutableVariant = response.mutable_variant();
    mutableVariant->set_id(boost::uuids::to_string(std::move(variant.id)));
    mutableVariant->set_question_id(boost::uuids::to_string(std::move(variant.question_id)));
    mutableVariant->set_text(std::move(variant.text));
    mutableVariant->set_is_correct(variant.is_correct);

    return response;
}

Service::GetVariantsByQuestionIdResult Service::GetVariantsByQuestionId(
      CallContext&
    , handlers::api::GetVariantsByQuestionIdRequest&& request
) {
    auto question_id = NUtils::StringToUuid(request.question_id());
    if (question_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.question_id()
        };
    }
    auto variants = NStorage::GetVariantsByQuestionId(
        pg_cluster_,
        question_id
    );

    handlers::api::GetVariantsByQuestionIdResponse response;
    auto mutableVariants = response.mutable_variants();

    for (auto&& variant : variants) {
        auto newVariant = mutableVariants->Add();
        newVariant->set_id(boost::uuids::to_string(variant.id));
        newVariant->set_question_id(boost::uuids::to_string(variant.question_id));
        newVariant->set_text(std::move(variant.text));
        newVariant->set_is_correct(variant.is_correct);
    }

    return response;
}

Service::CreateGameSessionResult Service::CreateGameSession(
      CallContext&
    , handlers::api::CreateGameSessionRequest&& request
) {
    auto pack_id = NUtils::StringToUuid(request.pack_id());
    if (pack_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.pack_id()
        };
    }
    
    auto createdGameSessionOpt = NStorage::CreateGameSession(pg_cluster_, pack_id);
    if (!createdGameSessionOpt.has_value()) {
        return grpc::Status{
            grpc::StatusCode::INTERNAL,
            "Failed to create game session"
        };
    }
    auto createdGameSession = createdGameSessionOpt.value();

    handlers::api::CreateGameSessionResponse response;
    auto mutableGameSession = response.mutable_game_session();
    mutableGameSession->set_id(boost::uuids::to_string(std::move(createdGameSession.id)));
    mutableGameSession->set_pack_id(boost::uuids::to_string(std::move(createdGameSession.pack_id)));
    mutableGameSession->set_state(std::move(createdGameSession.state));
    mutableGameSession->set_current_question_index(createdGameSession.current_question_index);
    
    // Convert time points to milliseconds since epoch
    auto created_at_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        createdGameSession.created_at.time_since_epoch()).count();
    mutableGameSession->set_created_at(created_at_ms);
    
    if (createdGameSession.started_at.has_value()) {
        auto started_at_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            createdGameSession.started_at.value().time_since_epoch()).count();
        mutableGameSession->set_started_at(started_at_ms);
    }
    
    if (createdGameSession.finished_at.has_value()) {
        auto finished_at_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            createdGameSession.finished_at.value().time_since_epoch()).count();
        mutableGameSession->set_finished_at(finished_at_ms);
    }

    return response;
}

Service::GetGameSessionResult Service::GetGameSession(
      CallContext&
    , handlers::api::GetGameSessionRequest&& request
) {
    auto game_session_id = NUtils::StringToUuid(request.id());
    if (game_session_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.id()
        };
    }
    
    auto gameSessionOpt = NStorage::GetGameSessionById(pg_cluster_, game_session_id);
    if (!gameSessionOpt.has_value()) {
        return grpc::Status{
            grpc::StatusCode::NOT_FOUND,
            "Game session not found"
        };
    }
    auto gameSession = gameSessionOpt.value();

    handlers::api::GetGameSessionResponse response;
    auto mutableGameSession = response.mutable_game_session();
    mutableGameSession->set_id(boost::uuids::to_string(std::move(gameSession.id)));
    mutableGameSession->set_pack_id(boost::uuids::to_string(std::move(gameSession.pack_id)));
    mutableGameSession->set_state(std::move(gameSession.state));
    mutableGameSession->set_current_question_index(gameSession.current_question_index);
    
    // Convert time points to milliseconds since epoch
    auto created_at_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        gameSession.created_at.time_since_epoch()).count();
    mutableGameSession->set_created_at(created_at_ms);
    
    if (gameSession.started_at.has_value()) {
        auto started_at_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            gameSession.started_at.value().time_since_epoch()).count();
        mutableGameSession->set_started_at(started_at_ms);
    }
    
    if (gameSession.finished_at.has_value()) {
        auto finished_at_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            gameSession.finished_at.value().time_since_epoch()).count();
        mutableGameSession->set_finished_at(finished_at_ms);
    }

    return response;
}

Service::StartGameSessionResult Service::StartGameSession(
      CallContext&
    , handlers::api::StartGameSessionRequest&& request
) {
    auto game_session_id = NUtils::StringToUuid(request.id());
    if (game_session_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.id()
        };
    }
    
    auto gameSessionOpt = NStorage::StartGameSession(pg_cluster_, game_session_id);
    if (!gameSessionOpt.has_value()) {
        return grpc::Status{
            grpc::StatusCode::INTERNAL,
            "Failed to start game session"
        };
    }
    auto gameSession = gameSessionOpt.value();

    handlers::api::StartGameSessionResponse response;
    auto mutableGameSession = response.mutable_game_session();
    mutableGameSession->set_id(boost::uuids::to_string(std::move(gameSession.id)));
    mutableGameSession->set_pack_id(boost::uuids::to_string(std::move(gameSession.pack_id)));
    mutableGameSession->set_state(std::move(gameSession.state));
    mutableGameSession->set_current_question_index(gameSession.current_question_index);
    
    // Convert time points to milliseconds since epoch
    auto created_at_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        gameSession.created_at.time_since_epoch()).count();
    mutableGameSession->set_created_at(created_at_ms);
    
    if (gameSession.started_at.has_value()) {
        auto started_at_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            gameSession.started_at.value().time_since_epoch()).count();
        mutableGameSession->set_started_at(started_at_ms);
    }
    
    if (gameSession.finished_at.has_value()) {
        auto finished_at_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            gameSession.finished_at.value().time_since_epoch()).count();
        mutableGameSession->set_finished_at(finished_at_ms);
    }

    return response;
}

Service::EndGameSessionResult Service::EndGameSession(
      CallContext&
    , handlers::api::EndGameSessionRequest&& request
) {
    auto game_session_id = NUtils::StringToUuid(request.id());
    if (game_session_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.id()
        };
    }
    
    auto gameSessionOpt = NStorage::EndGameSession(pg_cluster_, game_session_id);
    if (!gameSessionOpt.has_value()) {
        return grpc::Status{
            grpc::StatusCode::INTERNAL,
            "Failed to end game session"
        };
    }
    auto gameSession = gameSessionOpt.value();

    handlers::api::EndGameSessionResponse response;
    auto mutableGameSession = response.mutable_game_session();
    mutableGameSession->set_id(boost::uuids::to_string(std::move(gameSession.id)));
    mutableGameSession->set_pack_id(boost::uuids::to_string(std::move(gameSession.pack_id)));
    mutableGameSession->set_state(std::move(gameSession.state));
    mutableGameSession->set_current_question_index(gameSession.current_question_index);
    
    // Convert time points to milliseconds since epoch
    auto created_at_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        gameSession.created_at.time_since_epoch()).count();
    mutableGameSession->set_created_at(created_at_ms);
    
    if (gameSession.started_at.has_value()) {
        auto started_at_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            gameSession.started_at.value().time_since_epoch()).count();
        mutableGameSession->set_started_at(started_at_ms);
    }
    
    if (gameSession.finished_at.has_value()) {
        auto finished_at_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            gameSession.finished_at.value().time_since_epoch()).count();
        mutableGameSession->set_finished_at(finished_at_ms);
    }

    return response;
}

Service::GetGameSessionsResult Service::GetGameSessions(
      CallContext&
    , handlers::api::GetGameSessionsRequest&& request
) {
    auto gameSessions = NStorage::GetAllGameSessions(pg_cluster_);

    handlers::api::GetGameSessionsResponse response;
    auto mutableGameSessions = response.mutable_game_sessions();

    for (auto&& gameSession : gameSessions) {
        auto newGameSession = mutableGameSessions->Add();
        newGameSession->set_id(boost::uuids::to_string(gameSession.id));
        newGameSession->set_pack_id(boost::uuids::to_string(gameSession.pack_id));
        newGameSession->set_state(gameSession.state);
        newGameSession->set_current_question_index(gameSession.current_question_index);
        
        // Convert time points to milliseconds since epoch
        auto created_at_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            gameSession.created_at.time_since_epoch()).count();
        newGameSession->set_created_at(created_at_ms);
        
        if (gameSession.started_at.has_value()) {
            auto started_at_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                gameSession.started_at.value().time_since_epoch()).count();
            newGameSession->set_started_at(started_at_ms);
        }
        
        if (gameSession.finished_at.has_value()) {
            auto finished_at_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                gameSession.finished_at.value().time_since_epoch()).count();
            newGameSession->set_finished_at(finished_at_ms);
        }
    }

    return response;
}

Service::AddPlayerResult Service::AddPlayer(
      CallContext&
    , handlers::api::AddPlayerRequest&& request
) {
    auto game_session_id = NUtils::StringToUuid(request.game_session_id());
    if (game_session_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.game_session_id()
        };
    }
    
    if (request.name().empty()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Player name cannot be empty"
        };
    }
    
    auto playerOpt = NStorage::AddPlayer(pg_cluster_, game_session_id, request.name());
    if (!playerOpt.has_value()) {
        return grpc::Status{
            grpc::StatusCode::INTERNAL,
            "Failed to add player"
        };
    }
    auto player = playerOpt.value();

    handlers::api::AddPlayerResponse response;
    auto mutablePlayer = response.mutable_player();
    mutablePlayer->set_id(boost::uuids::to_string(std::move(player.id)));
    mutablePlayer->set_game_session_id(boost::uuids::to_string(std::move(player.game_session_id)));
    mutablePlayer->set_name(std::move(player.name));
    mutablePlayer->set_score(player.score);
    
    // Convert time point to milliseconds since epoch
    auto joined_at_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        player.joined_at.time_since_epoch()).count();
    mutablePlayer->set_joined_at(joined_at_ms);

    return response;
}

Service::GetPlayersResult Service::GetPlayers(
      CallContext&
    , handlers::api::GetPlayersRequest&& request
) {
    auto game_session_id = NUtils::StringToUuid(request.game_session_id());
    if (game_session_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.game_session_id()
        };
    }
    
    auto players = NStorage::GetPlayersByGameSessionId(pg_cluster_, game_session_id);

    handlers::api::GetPlayersResponse response;
    auto mutablePlayers = response.mutable_players();

    for (auto&& player : players) {
        auto newPlayer = mutablePlayers->Add();
        newPlayer->set_id(boost::uuids::to_string(player.id));
        newPlayer->set_game_session_id(boost::uuids::to_string(player.game_session_id));
        newPlayer->set_name(player.name);
        newPlayer->set_score(player.score);
        
        // Convert time point to milliseconds since epoch
        auto joined_at_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            player.joined_at.time_since_epoch()).count();
        newPlayer->set_joined_at(joined_at_ms);
    }

    return response;
}

Service::SubmitAnswerResult Service::SubmitAnswer(
      CallContext&
    , handlers::api::SubmitAnswerRequest&& request
) {
    auto player_id = NUtils::StringToUuid(request.player_id());
    if (player_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.player_id()
        };
    }
    
    auto question_id = NUtils::StringToUuid(request.question_id());
    if (question_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.question_id()
        };
    }
    
    auto variant_id = NUtils::StringToUuid(request.variant_id());
    if (variant_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.variant_id()
        };
    }
    
    // Check if the variant is correct
    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kSlave,
        samples_postgres_service::sql::kCheckVariantCorrectnessById,
        variant_id
    );
    
    bool is_correct = false;
    if (result.IsEmpty()) {
        return grpc::Status{
            grpc::StatusCode::NOT_FOUND,
            "Variant not found"
        };
    }
    
    std::tuple<bool> row;
    result.Front().To(row, userver::storages::postgres::kRowTag);
    is_correct = std::get<0>(row);
    
    // Submit the player's answer
    auto playerAnswerOpt = NStorage::SubmitPlayerAnswer(
        pg_cluster_, player_id, question_id, variant_id, is_correct);
    
    if (!playerAnswerOpt.has_value()) {
        return grpc::Status{
            grpc::StatusCode::INTERNAL,
            "Failed to submit answer"
        };
    }
    
    // Update player score if answer is correct
    int points = 0;
    if (is_correct) {
        points = 10; // Fixed points for correct answer
        auto playerOpt = NStorage::UpdatePlayerScore(pg_cluster_, player_id, points);
        if (!playerOpt.has_value()) {
            // Log error but don't fail the request
        }
    }

    handlers::api::SubmitAnswerResponse response;
    response.set_is_correct(is_correct);
    response.set_points(points);

    return response;
}

Service::GetPlayerAnswersResult Service::GetPlayerAnswers(
      CallContext&
    , handlers::api::GetPlayerAnswersRequest&& request
) {
    auto player_id = NUtils::StringToUuid(request.player_id());
    if (player_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.player_id()
        };
    }
    
    auto playerAnswers = NStorage::GetPlayerAnswersByPlayerId(pg_cluster_, player_id);

    handlers::api::GetPlayerAnswersResponse response;
    auto mutablePlayerAnswers = response.mutable_answers();

    for (auto&& playerAnswer : playerAnswers) {
        auto newPlayerAnswer = mutablePlayerAnswers->Add();
        newPlayerAnswer->set_id(boost::uuids::to_string(playerAnswer.id));
        newPlayerAnswer->set_player_id(boost::uuids::to_string(playerAnswer.player_id));
        newPlayerAnswer->set_question_id(boost::uuids::to_string(playerAnswer.question_id));
        newPlayerAnswer->set_variant_id(boost::uuids::to_string(playerAnswer.variant_id));
        newPlayerAnswer->set_is_correct(playerAnswer.is_correct);
        
        // Convert time point to milliseconds since epoch
        auto answered_at_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            playerAnswer.answered_at.time_since_epoch()).count();
        newPlayerAnswer->set_answered_at(answered_at_ms);
    }

    return response;
}

} // namespace game_userver
