#include "create_variant.hpp"

#include <sql_queries/sql_queries.hpp>
#include <stdexcept>
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

#include "models/variant.hpp"
#include "storage/variants.hpp"
#include "utils/constants.hpp"
#include "utils/string_to_uuid.hpp"
#include "utils/variant.hpp"

namespace game_userver {

struct CreateVariant::Impl {
    userver::storages::postgres::ClusterPtr pg_cluster;

    explicit Impl(const userver::components::ComponentContext& context)
        : pg_cluster(context
                         .FindComponent<userver::components::Postgres>(
                             Constants::kDatabaseName
                         )
                         .GetCluster()) {}
};

CreateVariant::CreateVariant(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context), impl_(component_context) {}

CreateVariant::~CreateVariant() = default;

auto CreateVariant::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
    /*context*/
) const -> std::string {
    auto variantData = Utils::GetVariantDataFromRequest(request);
    const auto& question_id_str = request.GetPathArg("question_id");
    variantData.question_id = Utils::StringToUuid(question_id_str);

    const auto createdVariantOpt =
        NStorage::CreateVariant(impl_->pg_cluster, variantData);

    if (!createdVariantOpt) {
        request.GetHttpResponse().SetStatus(
            userver::server::http::HttpStatus::kInternalServerError
        );
        throw std::runtime_error("Failed to create variant");
    }

    return userver::formats::json::ToPrettyString(
        userver::formats::json::ValueBuilder{createdVariantOpt.value()}
            .ExtractValue()
    );
}

} // namespace game_userver
