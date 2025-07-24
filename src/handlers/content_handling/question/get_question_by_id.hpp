#pragma once

#include <userver/components/component.hpp>
#include <userver/server/handlers/http_handler_base.hpp>

#include <userver/storages/postgres/cluster.hpp>

namespace game_userver {

class GetQuestionById final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-get-question-by-id";

    GetQuestionById(
          const userver::components::ComponentConfig&
        , const userver::components::ComponentContext&
    );

    std::string HandleRequestThrow(
          const userver::server::http::HttpRequest&
        , userver::server::request::RequestContext&
    ) const override;

private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};

} // namespace game_userver
