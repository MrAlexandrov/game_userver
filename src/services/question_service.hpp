#pragma once

#include <optional>
#include <string>
#include <vector>

#include <boost/uuid/uuid.hpp>
#include <userver/components/loggable_component_base.hpp>
#include <userver/storages/postgres/postgres_fwd.hpp>

#include "models/question.hpp"

namespace game_userver::services {

class QuestionService final
    : public userver::components::LoggableComponentBase {
public:
    static constexpr std::string_view kName = "question-service";

    QuestionService(
        const userver::components::ComponentConfig&,
        const userver::components::ComponentContext&
    );

    ~QuestionService() override;

    // Business logic methods
    std::optional<Models::Question> CreateQuestion(
        const boost::uuids::uuid& pack_id, const std::string& text,
        const std::string& image_url
    );

    std::optional<Models::Question>
    GetQuestionById(const boost::uuids::uuid& question_id);

    std::vector<Models::Question>
    GetQuestionsByPackId(const boost::uuids::uuid& pack_id);

private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};

} // namespace game_userver::services
