#include "question_service.hpp"

#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

#include "storage/questions.hpp"
#include "utils/constants.hpp"

namespace game_userver::services {

QuestionService::QuestionService(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : LoggableComponentBase(config, context),
      pg_cluster_(context
                      .FindComponent<userver::components::Postgres>(
                          Constants::kDatabaseName
                      )
                      .GetCluster()) {}

QuestionService::~QuestionService() = default;

std::optional<Models::Question> QuestionService::CreateQuestion(
    const boost::uuids::uuid& pack_id, const std::string& text,
    const std::string& image_url
) {
    return NStorage::CreateQuestion(pg_cluster_, pack_id, text, image_url);
}

std::optional<Models::Question>
QuestionService::GetQuestionById(const boost::uuids::uuid& question_id) {
    return NStorage::GetQuestionById(pg_cluster_, question_id);
}

std::vector<Models::Question>
QuestionService::GetQuestionsByPackId(const boost::uuids::uuid& pack_id) {
    return NStorage::GetQuestionsByPackId(pg_cluster_, pack_id);
}

} // namespace game_userver::services
