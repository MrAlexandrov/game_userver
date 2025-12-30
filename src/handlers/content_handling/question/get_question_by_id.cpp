#include "get_question_by_id.hpp"

#include "storage/questions.hpp"
#include "utils/string_to_uuid.hpp"

#include <samples_postgres_service/sql_queries.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

namespace game_userver {

struct GetQuestionById::Impl {
    userver::storages::postgres::ClusterPtr pg_cluster;

    explicit Impl(const userver::components::ComponentContext& context)
        : pg_cluster(
              context.FindComponent<userver::components::Postgres>("postgres-db-1").GetCluster()
          ) {}
};

GetQuestionById::GetQuestionById(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context),
      impl_(component_context) {}

GetQuestionById::~GetQuestionById() = default;

std::string GetQuestionById::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    const auto& stringId = request.GetArg("id");

    const auto id = NUtils::StringToUuid(stringId);
    if (id.is_nil()) {
        return "Incorrect id";
    }

    const auto questionOpt = NStorage::GetQuestionById(impl_->pg_cluster, id);
    if (!questionOpt) {
        return {};
    }

    return userver::formats::json::ToPrettyString(
        userver::formats::json::ValueBuilder{questionOpt.value()}.ExtractValue()
    );
}

} // namespace game_userver
