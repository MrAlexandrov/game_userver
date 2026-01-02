#pragma once

#include <userver/server/handlers/http_handler_base.hpp>

namespace game_userver {

class Hello final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-hello";

    using HttpHandlerBase::HttpHandlerBase;

    auto HandleRequestThrow(
        const userver::server::http::HttpRequest& /*request*/,
        userver::server::request::RequestContext&
        /*context*/
    ) const -> std::string override;
};

} // namespace game_userver
