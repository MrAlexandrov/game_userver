#include "create_pack.hpp"

#include "storage/packs.hpp"

#include <samples_postgres_service/sql_queries.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

#include <userver/logging/log.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace game_userver {

struct CreatePack::Impl {
    userver::storages::postgres::ClusterPtr pg_cluster;

    explicit Impl(const userver::components::ComponentContext& context)
        : pg_cluster(
              context.FindComponent<userver::components::Postgres>("postgres-db-1").GetCluster()
          ) {}
};

CreatePack::CreatePack(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context),
      impl_(component_context) {}

CreatePack::~CreatePack() = default;

std::string CreatePack::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
) const {
    using userver::logging::Level::kDebug;

    const auto& title = request.GetArg("title");
    LOG(kDebug) << "title: " << title;

    const auto createdPackOpt = NStorage::CreatePack(impl_->pg_cluster, title);
    if (!createdPackOpt) {
        request.GetHttpResponse().SetStatus(
            userver::server::http::HttpStatus::kInternalServerError
        );
        return {};
    }
    const auto& [id, pack_title] = createdPackOpt.value();

    LOG(kDebug) << "inserted pack:\n"
                << boost::uuids::to_string(id) << " " << pack_title;

    return userver::formats::json::ToPrettyString(
        userver::formats::json::ValueBuilder{createdPackOpt.value()}.ExtractValue()
    );
}

} // namespace game_userver
