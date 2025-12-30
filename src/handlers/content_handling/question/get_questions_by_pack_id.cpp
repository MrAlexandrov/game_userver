#include "get_questions_by_pack_id.hpp"

#include "storage/questions.hpp"
#include "utils/string_to_uuid.hpp"

#include <samples_postgres_service/sql_queries.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

namespace game_userver {

struct GetQuestionsByPackId::Impl {
    userver::storages::postgres::ClusterPtr pg_cluster;

    explicit Impl(const userver::components::ComponentContext& context)
        : pg_cluster(
              context
                  .FindComponent<userver::components::Postgres>("postgres-db-1")
                  .GetCluster()
          ) {}
};

GetQuestionsByPackId::GetQuestionsByPackId(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context), impl_(component_context) {}

GetQuestionsByPackId::~GetQuestionsByPackId() = default;

std::string GetQuestionsByPackId::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    const auto& stringPackId = request.GetArg("pack_id");

    const auto questions = NStorage::GetQuestionsByPackId(
        impl_->pg_cluster, NUtils::StringToUuid(stringPackId)
    );

    userver::formats::json::ValueBuilder result{
        userver::formats::common::Type::kArray
    };

    for (const auto& question : questions) {
        result.PushBack(question);
    }

    return userver::formats::json::ToPrettyString(result.ExtractValue());
}

} // namespace game_userver
