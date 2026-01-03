#include "get_question_by_id.hpp"

#include <sql_queries/sql_queries.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

#include "storage/questions.hpp"
#include "utils/constants.hpp"
#include "utils/string_to_uuid.hpp"

namespace game_userver {

struct GetQuestionById::Impl {
    userver::storages::postgres::ClusterPtr pg_cluster;

    explicit Impl(const userver::components::ComponentContext& context)
        : pg_cluster(context
                         .FindComponent<userver::components::Postgres>(
                             Constants::kDatabaseName
                         )
                         .GetCluster()) {}
};

GetQuestionById::GetQuestionById(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context), impl_(component_context) {}

GetQuestionById::~GetQuestionById() = default;

auto GetQuestionById::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
    /*context*/
) const -> std::string {
    const auto& stringId = request.GetPathArg("question_id");

    const auto id = Utils::StringToUuid(stringId);
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
