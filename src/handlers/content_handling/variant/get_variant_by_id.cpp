#include "get_variant_by_id.hpp"

#include <sql_queries/sql_queries.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

#include "storage/variants.hpp"
#include "utils/constants.hpp"
#include "utils/string_to_uuid.hpp"

namespace game_userver {

struct GetVariantById::Impl {
    userver::storages::postgres::ClusterPtr pg_cluster;

    explicit Impl(const userver::components::ComponentContext& context)
        : pg_cluster(context
                         .FindComponent<userver::components::Postgres>(
                             Constants::kDatabaseName
                         )
                         .GetCluster()) {}
};

GetVariantById::GetVariantById(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context), impl_(component_context) {}

GetVariantById::~GetVariantById() = default;

std::string GetVariantById::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    const auto& stringId = request.GetArg("id");

    const auto id = Utils::StringToUuid(stringId);
    if (id.is_nil()) {
        return "Incorrect id";
    }

    const auto variantOpt = NStorage::GetVariantById(impl_->pg_cluster, id);
    if (!variantOpt) {
        return {};
    }

    return userver::formats::json::ToPrettyString(
        userver::formats::json::ValueBuilder{variantOpt.value()}.ExtractValue()
    );
}

} // namespace game_userver
