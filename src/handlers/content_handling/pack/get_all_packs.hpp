#pragma once

#include <string>
#include <userver/components/component_fwd.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/utils/fast_pimpl.hpp>

namespace game_userver {

class GetAllPacks final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-get-all-packs";

    GetAllPacks(
        const userver::components::ComponentConfig&,
        const userver::components::ComponentContext&
    );
    ~GetAllPacks() override;

    auto HandleRequestThrow(
        const userver::server::http::HttpRequest& /*request*/,
        userver::server::request::RequestContext&
        /*context*/
    ) const -> std::string override;

private:
    struct Impl;
    static constexpr size_t kSize = 16;
    static constexpr size_t kAlignment = 8;
    userver::utils::FastPimpl<Impl, kSize, kAlignment> impl_;
};

} // namespace game_userver
