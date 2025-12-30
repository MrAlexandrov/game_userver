#include "create_question.hpp"

#include <samples_postgres_service/sql_queries.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

#include "storage/questions.hpp"
#include "utils/string_to_uuid.hpp"

namespace game_userver {

struct CreateQuestion::Impl {
    userver::storages::postgres::ClusterPtr pg_cluster;

    explicit Impl(const userver::components::ComponentContext& context)
        : pg_cluster(
              context
                  .FindComponent<userver::components::Postgres>("postgres-db-1")
                  .GetCluster()
          ) {}
};

CreateQuestion::CreateQuestion(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context), impl_(component_context) {}

CreateQuestion::~CreateQuestion() = default;

std::string CreateQuestion::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    const auto& pack_id = request.GetArg("pack_id");
    const auto& text = request.GetArg("text");
    const auto& image_url = request.GetArg("image_url");

    const auto createdQuestionOpt = NStorage::CreateQuestion(
        impl_->pg_cluster, NUtils::StringToUuid(pack_id), text, image_url
    );

    return userver::formats::json::ToPrettyString(
        userver::formats::json::ValueBuilder{createdQuestionOpt.value()}
            .ExtractValue()
    );
}

} // namespace game_userver
