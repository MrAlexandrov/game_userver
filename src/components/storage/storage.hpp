#pragma once

#include <string_view>

#include <boost/uuid/uuid.hpp>
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

    [[nodiscard]] auto GetPackById(const boost::uuids::uuid& pack_id) const
        -> std::optional<Models::Pack>;

    [[nodiscard]] auto GetAllPacks() const -> std::vector<Models::Pack>;

    // ===== Questions =====
    [[nodiscard]] auto CreateQuestion(Models::Question&& question) const
        -> std::optional<Models::Question>;

    [[nodiscard]] auto
    GetQuestionById(const boost::uuids::uuid& question_id) const
        -> std::optional<Models::Question>;

    [[nodiscard]] auto
    GetQuestionsByPackId(const boost::uuids::uuid& pack_id) const
        -> std::vector<Models::Question>;

    // ===== Variants =====
    [[nodiscard]] auto CreateVariant(const Models::Variant& variant) const
        -> std::optional<Models::Variant>;

    [[nodiscard]] auto
    GetVariantById(const boost::uuids::uuid& variant_id) const
        -> std::optional<Models::Variant>;

    [[nodiscard]] auto
    GetVariantsByQuestionId(const boost::uuids::uuid& question_id) const
        -> std::vector<Models::Variant>;

    [[nodiscard]] auto
    CheckVariantCorrectness(const boost::uuids::uuid& variant_id) const
        -> std::optional<bool>;

    // ===== Game Sessions =====
    [[nodiscard]] auto
    CreateGameSession(const boost::uuids::uuid& pack_id) const
        -> std::optional<Models::GameSession>;

    [[nodiscard]] auto
    GetGameSessionById(const boost::uuids::uuid& game_session_id) const
        -> std::optional<Models::GameSession>;

    [[nodiscard]] auto
    StartGameSession(const boost::uuids::uuid& game_session_id) const
        -> std::optional<Models::GameSession>;

    [[nodiscard]] auto AdvanceToNextQuestion(
        const boost::uuids::uuid& game_session_id, int current_question_index
    ) const -> std::optional<Models::GameSession>;

    [[nodiscard]] auto
    EndGameSession(const boost::uuids::uuid& game_session_id) const
        -> std::optional<Models::GameSession>;

    [[nodiscard]] auto GetAllGameSessions() const
        -> std::vector<Models::GameSession>;

    // ===== Players =====
    [[nodiscard]] auto AddPlayer(
        const boost::uuids::uuid& game_session_id,
        const std::string& player_name
    ) const -> std::optional<Models::Player>;

    [[nodiscard]] auto GetPlayerById(const boost::uuids::uuid& player_id) const
        -> std::optional<Models::Player>;

    [[nodiscard]] auto
    GetPlayersByGameSessionId(const boost::uuids::uuid& game_session_id) const
        -> std::vector<Models::Player>;

    [[nodiscard]] auto UpdatePlayerScore(
        const boost::uuids::uuid& player_id, int score_delta
    ) const -> std::optional<Models::Player>;

    // ===== Player Answers =====
    [[nodiscard]] auto SubmitPlayerAnswer(
        const boost::uuids::uuid& player_id,
        const boost::uuids::uuid& question_id,
        const std::optional<boost::uuids::uuid>& variant_id,
        const std::optional<std::string>& text_answer, bool is_correct
    ) const -> std::optional<Models::PlayerAnswer>;

    [[nodiscard]] auto
    GetPlayerAnswersByPlayerId(const boost::uuids::uuid& player_id) const
        -> std::vector<Models::PlayerAnswer>;

    // ===== Text Answers =====
    [[nodiscard]] auto
    CreateTextAnswer(const Models::TextAnswer& text_answer) const
        -> std::optional<Models::TextAnswer>;

    [[nodiscard]] auto
    GetTextAnswerById(const boost::uuids::uuid& text_answer_id) const
        -> std::optional<Models::TextAnswer>;

    [[nodiscard]] auto
    GetTextAnswersByQuestionId(const boost::uuids::uuid& question_id) const
        -> std::vector<Models::TextAnswer>;

    [[nodiscard]] auto GetAnswersCountForQuestion(
        const boost::uuids::uuid& game_session_id,
        const boost::uuids::uuid& question_id
    ) const -> int;

    // ===== Questions and Variants =====
    [[nodiscard]] auto
    GetQuestionsAndVariantsByPackId(const boost::uuids::uuid& pack_id) const
        -> std::vector<
            std::pair<Models::Question, std::vector<Models::Variant>>>;

    [[nodiscard]] auto GetCluster() const
        -> userver::storages::postgres::ClusterPtr;

private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};

} // namespace game_userver::components
