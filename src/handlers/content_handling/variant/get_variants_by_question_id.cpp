#include "get_variants_by_question_id.hpp"

#include <sql_queries/sql_queries.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

#include "storage/variants.hpp"
#include "utils/string_to_uuid.hpp"
#include "utils/constants.hpp"

namespace game_userver {

struct GetVariantsByQuestionId::Impl {
    userver::storages::postgres::ClusterPtr pg_cluster;

    explicit Impl(const userver::components::ComponentContext& context)
        : pg_cluster(
              context
                  .FindComponent<userver::components::Postgres>(Constants::kDatabaseName)
                  .GetCluster()
          ) {}
};

GetVariantsByQuestionId::GetVariantsByQuestionId(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context), impl_(component_context) {}

GetVariantsByQuestionId::~GetVariantsByQuestionId() = default;

std::string GetVariantsByQuestionId::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    const auto& stringQuestionId = request.GetArg("question_id");

    const auto variants = NStorage::GetVariantsByQuestionId(
        impl_->pg_cluster, NUtils::StringToUuid(stringQuestionId)
    );

    userver::formats::json::ValueBuilder result{
        userver::formats::common::Type::kArray
    };

    for (const auto& variant : variants) {
        result.PushBack(variant);
    }

    return userver::formats::json::ToPrettyString(result.ExtractValue());
}

} // namespace game_userver
