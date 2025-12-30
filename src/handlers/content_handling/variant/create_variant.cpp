#include "create_variant.hpp"

#include "storage/variants.hpp"
#include "utils/string_to_uuid.hpp"

#include <samples_postgres_service/sql_queries.hpp>
#include <userver/storages/postgres/component.hpp>

#include <userver/logging/log.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace game_userver {

CreateVariant::CreateVariant(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context),
      pg_cluster_(
          component_context
              .FindComponent<userver::components::Postgres>("postgres-db-1")
              .GetCluster()
      ) {}

std::string CreateVariant::
    HandleRequestThrow(const userver::server::http::HttpRequest& request, userver::server::request::RequestContext&)
        const {
    using userver::logging::Level::kDebug;

    const auto& question_id = request.GetArg("question_id");
    const auto& text = request.GetArg("text");
    const auto& is_correct = request.GetArg("is_correct");

    const auto createdVariantOpt = NStorage::CreateVariant(
        pg_cluster_, NUtils::StringToUuid(question_id), text,
        NUtils::StringToBool(is_correct)
    );
    const auto createdVariant = createdVariantOpt.value();

    return userver::formats::json::ToPrettyString(
        userver::formats::json::ValueBuilder{createdVariant}.ExtractValue()
    );
}

} // namespace game_userver
