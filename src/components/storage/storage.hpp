#pragma once

#include <string_view>

#include <userver/components/component.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

#include "models/game_session.hpp"
#include "models/pack.hpp"
#include "models/player.hpp"
#include "models/player_answer.hpp"
#include "models/question.hpp"
#include "models/text_answer.hpp"
#include "models/variant.hpp"

namespace game_userver::components {

class Storage final : public userver::components::LoggableComponentBase {
public:
    static constexpr std::string_view kName = "storage";

    Storage(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    );

    ~Storage() override;

    // ===== Packs =====
    [[nodiscard]] auto CreatePack(const Models::Pack& pack) const
        -> std::optional<Models::Pack>;

    [[nodiscard]] auto GetPackById(const Models::Pack::PackId& pack_id) const
        -> std::optional<Models::Pack>;

    [[nodiscard]] auto GetAllPacks() const -> std::vector<Models::Pack>;

    // ===== Questions =====
    [[nodiscard]] auto CreateQuestion(Models::Question&& question) const
        -> std::optional<Models::Question>;

    [[nodiscard]] auto
    GetQuestionById(const Models::Question::QuestionId& question_id) const
        -> std::optional<Models::Question>;

    [[nodiscard]] auto
    GetQuestionsByPackId(const Models::Pack::PackId& pack_id) const
        -> std::vector<Models::Question>;

    // ===== Variants =====
    [[nodiscard]] auto CreateVariant(const Models::Variant& variant) const
        -> std::optional<Models::Variant>;

    [[nodiscard]] auto
    GetVariantById(const Models::Variant::VariantId& variant_id) const
        -> std::optional<Models::Variant>;

    [[nodiscard]] auto GetVariantsByQuestionId(
        const Models::Question::QuestionId& question_id
    ) const -> std::vector<Models::Variant>;

    [[nodiscard]] auto
    CheckVariantCorrectness(const Models::Variant::VariantId& variant_id) const
        -> std::optional<bool>;

    // ===== Game Sessions =====
    [[nodiscard]] auto
    CreateGameSession(const Models::Pack::PackId& pack_id) const
        -> std::optional<Models::GameSession>;

    [[nodiscard]] auto GetGameSessionById(
        const Models::GameSession::GameSessionId& game_session_id
    ) const -> std::optional<Models::GameSession>;

    [[nodiscard]] auto StartGameSession(
        const Models::GameSession::GameSessionId& game_session_id
    ) const -> std::optional<Models::GameSession>;

    [[nodiscard]] auto AdvanceToNextQuestion(
        const Models::GameSession::GameSessionId& game_session_id,
        int current_question_index
    ) const -> std::optional<Models::GameSession>;

    [[nodiscard]] auto EndGameSession(
        const Models::GameSession::GameSessionId& game_session_id
    ) const -> std::optional<Models::GameSession>;

    [[nodiscard]] auto GetAllGameSessions() const
        -> std::vector<Models::GameSession>;

    // ===== Players =====
    [[nodiscard]] auto AddPlayer(
        const Models::GameSession::GameSessionId& game_session_id,
        const std::string& player_name
    ) const -> std::optional<Models::Player>;

    [[nodiscard]] auto
    GetPlayerById(const Models::Player::PlayerId& player_id) const
        -> std::optional<Models::Player>;

    [[nodiscard]] auto GetPlayersByGameSessionId(
        const Models::GameSession::GameSessionId& game_session_id
    ) const -> std::vector<Models::Player>;

    [[nodiscard]] auto UpdatePlayerScore(
        const Models::Player::PlayerId& player_id, int score_delta
    ) const -> std::optional<Models::Player>;

    // ===== Player Answers =====
    [[nodiscard]] auto SubmitPlayerAnswer(
        const Models::Player::PlayerId& player_id,
        const Models::Question::QuestionId& question_id,
        const std::optional<Models::Variant::VariantId>& variant_id,
        const std::optional<std::string>& text_answer, bool is_correct
    ) const -> std::optional<Models::PlayerAnswer>;

    [[nodiscard]] auto
    GetPlayerAnswersByPlayerId(const Models::Player::PlayerId& player_id) const
        -> std::vector<Models::PlayerAnswer>;

    // ===== Text Answers =====
    [[nodiscard]] auto
    CreateTextAnswer(const Models::TextAnswer& text_answer) const
        -> std::optional<Models::TextAnswer>;

    [[nodiscard]] auto GetTextAnswerById(
        const Models::TextAnswer::TextAnswerId& text_answer_id
    ) const -> std::optional<Models::TextAnswer>;

    [[nodiscard]] auto GetTextAnswersByQuestionId(
        const Models::Question::QuestionId& question_id
    ) const -> std::vector<Models::TextAnswer>;

    [[nodiscard]] auto GetAnswersCountForQuestion(
        const Models::GameSession::GameSessionId& game_session_id,
        const Models::Question::QuestionId& question_id
    ) const -> int;

    // ===== Questions and Variants =====
    [[nodiscard]] auto
    GetQuestionsAndVariantsByPackId(const Models::Pack::PackId& pack_id) const
        -> std::vector<
            std::pair<Models::Question, std::vector<Models::Variant>>>;

    [[nodiscard]] auto GetCluster() const
        -> userver::storages::postgres::ClusterPtr;

private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};

} // namespace game_userver::components
