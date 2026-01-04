#include "create_pack.hpp"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <sql_queries/sql_queries.hpp>
#include <userver/components/component_context.hpp>
#include <userver/logging/log.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

#include "models/pack.hpp"
#include "storage/packs.hpp"

#include "utils/constants.hpp"
#include "utils/pack.hpp"

namespace game_userver {

struct CreatePack::Impl {
    userver::storages::postgres::ClusterPtr pg_cluster;

    explicit Impl(const userver::components::ComponentContext& context)
        : pg_cluster(context
                         .FindComponent<userver::components::Postgres>(
                             Constants::kDatabaseName
                         )
                         .GetCluster()) {}
};

CreatePack::CreatePack(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context), impl_(component_context) {}

CreatePack::~CreatePack() = default;

auto CreatePack::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
    /*context*/
) const -> std::string {
    using userver::logging::Level::kDebug;

    const auto pack = Utils::GetPackFromRequest(request);

    const auto createdPackOpt = NStorage::CreatePack(impl_->pg_cluster, pack);
    if (!createdPackOpt) {
        request.GetHttpResponse().SetStatus(
            userver::server::http::HttpStatus::kInternalServerError
        );
        throw std::runtime_error("Failed to create pack");
    }
    const auto& [id, title] = createdPackOpt.value();

    LOG(kDebug) << "inserted pack:\n"
                << boost::uuids::to_string(id) << " " << title;

    return userver::formats::json::ToPrettyString(
        userver::formats::json::ValueBuilder{createdPackOpt.value()}
            .ExtractValue()
    );
}

} // namespace game_userver
