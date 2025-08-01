#pragma once

#include <userver/server/handlers/http_handler_base.hpp>

namespace game_userver {

class Hello final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-hello";

    using HttpHandlerBase::HttpHandlerBase;

    std::string HandleRequestThrow(
          const userver::server::http::HttpRequest&
        , userver::server::request::RequestContext&
    ) const override;
};

} // namespace game_userver
