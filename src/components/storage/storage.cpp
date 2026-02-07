#include "storage.hpp"

#include <userver/components/component_context.hpp>
#include <userver/logging/log.hpp>

#include "storage/game_sessions.hpp"
#include "storage/packs.hpp"
#include "storage/player_answers.hpp"
#include "storage/players.hpp"
#include "storage/questions.hpp"
#include "storage/questions_and_variants.hpp"
#include "storage/text_answers.hpp"
#include "storage/variants.hpp"
#include "utils/constants.hpp"

namespace game_userver::components {

Storage::Storage(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : ComponentBase(config, context),
      pg_cluster_(context
                      .FindComponent<userver::components::Postgres>(
                          Constants::kDatabaseName
                      )
                      .GetCluster()) {}

Storage::~Storage() {}

// ===== Packs =====

auto Storage::CreatePack(const Models::Pack& pack)
    -> std::optional<Models::Pack> {
    return NStorage::CreatePack(pg_cluster_, pack);
}

auto Storage::GetPackById(const boost::uuids::uuid& pack_id)
    -> std::optional<Models::Pack> {
    return NStorage::GetPackById(pg_cluster_, pack_id);
}

auto Storage::GetAllPacks() -> std::vector<Models::Pack> {
    return NStorage::GetAllPacks(pg_cluster_);
}

// ===== Questions =====

auto Storage::CreateQuestion(Models::Question&& question)
    -> std::optional<Models::Question> {
    return NStorage::CreateQuestion(pg_cluster_, std::move(question));
}

auto Storage::GetQuestionById(const boost::uuids::uuid& question_id)
    -> std::optional<Models::Question> {
    return NStorage::GetQuestionById(pg_cluster_, question_id);
}

auto Storage::GetQuestionsByPackId(const boost::uuids::uuid& pack_id)
    -> std::vector<Models::Question> {
    return NStorage::GetQuestionsByPackId(pg_cluster_, pack_id);
}

// ===== Variants =====

auto Storage::CreateVariant(const Models::Variant& variant)
    -> std::optional<Models::Variant> {
    return NStorage::CreateVariant(pg_cluster_, variant);
}

auto Storage::GetVariantById(const boost::uuids::uuid& variant_id)
    -> std::optional<Models::Variant> {
    return NStorage::GetVariantById(pg_cluster_, variant_id);
}

auto Storage::GetVariantsByQuestionId(const boost::uuids::uuid& question_id)
    -> std::vector<Models::Variant> {
    return NStorage::GetVariantsByQuestionId(pg_cluster_, question_id);
}

auto Storage::CheckVariantCorrectness(const boost::uuids::uuid& variant_id)
    -> std::optional<bool> {
    return NStorage::CheckVariantCorrectnessById(pg_cluster_, variant_id);
}

// ===== Game Sessions =====

auto Storage::CreateGameSession(const boost::uuids::uuid& pack_id)
    -> std::optional<Models::GameSession> {
    return NStorage::CreateGameSession(pg_cluster_, pack_id);
}

auto Storage::GetGameSessionById(const boost::uuids::uuid& game_session_id)
    -> std::optional<Models::GameSession> {
    return NStorage::GetGameSessionById(pg_cluster_, game_session_id);
}

auto Storage::StartGameSession(const boost::uuids::uuid& game_session_id)
    -> std::optional<Models::GameSession> {
    return NStorage::StartGameSession(pg_cluster_, game_session_id);
}

auto Storage::AdvanceToNextQuestion(
    const boost::uuids::uuid& game_session_id, int current_question_index
) -> std::optional<Models::GameSession> {
    return NStorage::AdvanceToNextQuestion(
        pg_cluster_, game_session_id, current_question_index
    );
}

auto Storage::EndGameSession(const boost::uuids::uuid& game_session_id)
    -> std::optional<Models::GameSession> {
    return NStorage::EndGameSession(pg_cluster_, game_session_id);
}

auto Storage::GetAllGameSessions() -> std::vector<Models::GameSession> {
    return NStorage::GetAllGameSessions(pg_cluster_);
}

// ===== Players =====

auto Storage::AddPlayer(
    const boost::uuids::uuid& game_session_id, const std::string& player_name
) -> std::optional<Models::Player> {
    return NStorage::AddPlayer(pg_cluster_, game_session_id, player_name);
}

auto Storage::GetPlayerById(const boost::uuids::uuid& player_id)
    -> std::optional<Models::Player> {
    return NStorage::GetPlayerById(pg_cluster_, player_id);
}

auto Storage::GetPlayersByGameSessionId(
    const boost::uuids::uuid& game_session_id
) -> std::vector<Models::Player> {
    return NStorage::GetPlayersByGameSessionId(pg_cluster_, game_session_id);
}

auto Storage::UpdatePlayerScore(
    const boost::uuids::uuid& player_id, int score_delta
) -> std::optional<Models::Player> {
    return NStorage::UpdatePlayerScore(pg_cluster_, player_id, score_delta);
}

// ===== Player Answers =====

auto Storage::SubmitPlayerAnswer(
    const boost::uuids::uuid& player_id, const boost::uuids::uuid& question_id,
    const std::optional<boost::uuids::uuid>& variant_id,
    const std::optional<std::string>& text_answer, bool is_correct
) -> std::optional<Models::PlayerAnswer> {
    return NStorage::SubmitPlayerAnswer(
        pg_cluster_, player_id, question_id, variant_id, text_answer, is_correct
    );
}

auto Storage::GetPlayerAnswersByPlayerId(const boost::uuids::uuid& player_id)
    -> std::vector<Models::PlayerAnswer> {
    return NStorage::GetPlayerAnswersByPlayerId(pg_cluster_, player_id);
}

// ===== Text Answers =====

auto Storage::CreateTextAnswer(const Models::TextAnswer& text_answer)
    -> std::optional<Models::TextAnswer> {
    return NStorage::CreateTextAnswer(pg_cluster_, text_answer);
}

auto Storage::GetTextAnswerById(const boost::uuids::uuid& text_answer_id)
    -> std::optional<Models::TextAnswer> {
    return NStorage::GetTextAnswerById(pg_cluster_, text_answer_id);
}

auto Storage::GetTextAnswersByQuestionId(const boost::uuids::uuid& question_id)
    -> std::vector<Models::TextAnswer> {
    return NStorage::GetTextAnswersByQuestionId(pg_cluster_, question_id);
}

auto Storage::GetAnswersCountForQuestion(
    const boost::uuids::uuid& game_session_id,
    const boost::uuids::uuid& question_id
) -> int {
    return NStorage::GetAnswersCountForQuestion(
        pg_cluster_, game_session_id, question_id
    );
}

// ===== Questions and Variants =====

auto Storage::GetQuestionsAndVariantsByPackId(const boost::uuids::uuid& pack_id)
    -> std::vector<std::pair<Models::Question, std::vector<Models::Variant>>> {
    return NStorage::GetQuestionsAndVariantsByPackId(pg_cluster_, pack_id);
}

// ===== Utility =====

userver::storages::postgres::ClusterPtr Storage::GetCluster() const {
    return pg_cluster_;
}

} // namespace game_userver::components
