#pragma once

#include <string>
#include <userver/server/handlers/http_handler_base.hpp>

namespace game_userver {

namespace components {
class Storage;
} // namespace components

class GetPack final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-get-pack-by-id";

    GetPack(
        const userver::components::ComponentConfig&,
        const userver::components::ComponentContext&
    );
    ~GetPack() override;

    auto HandleRequestThrow(
        const userver::server::http::HttpRequest& /*request*/,
        userver::server::request::RequestContext&
        /*context*/
    ) const -> std::string override;

private:
    const components::Storage& storage_;
};

} // namespace game_userver
