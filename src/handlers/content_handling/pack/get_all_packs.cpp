#include "get_all_packs.hpp"

#include <sql_queries/sql_queries.hpp>
#include <userver/components/component_context.hpp>
#include <userver/logging/log.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

#include "components/storage/storage.hpp"

namespace game_userver {

GetAllPacks::GetAllPacks(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context
)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<components::Storage>()) {}

GetAllPacks::~GetAllPacks() = default;

auto GetAllPacks::HandleRequestThrow(
    const userver::server::http::HttpRequest& /*request*/,
    userver::server::request::RequestContext&
    /*context*/
) const -> std::string {
    const auto packs = storage_.GetAllPacks();

    userver::formats::json::ValueBuilder result{
        userver::formats::common::Type::kArray
    };

    for (const auto& pack : packs) {
        result.PushBack(pack);
    }

    return userver::formats::json::ToPrettyString(result.ExtractValue());
}

} // namespace game_userver
