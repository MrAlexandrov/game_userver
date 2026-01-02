#include "create_variant.hpp"

#include <sql_queries/sql_queries.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

#include "storage/variants.hpp"
#include "utils/constants.hpp"
#include "utils/string_to_uuid.hpp"

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
    const auto& question_id = request.GetArg("question_id");
    const auto& text = request.GetArg("text");
    const auto& is_correct = request.GetArg("is_correct");

    const auto createdVariantOpt = NStorage::CreateVariant(
        impl_->pg_cluster, Utils::StringToUuid(question_id), text,
        Utils::StringToBool(is_correct)
    );

    if (!createdVariantOpt) {
        request.GetHttpResponse().SetStatus(
            userver::server::http::HttpStatus::kInternalServerError
        );
        return {};
    }

    return userver::formats::json::ToPrettyString(
        userver::formats::json::ValueBuilder{createdVariantOpt.value()}
            .ExtractValue()
    );
}

} // namespace game_userver
