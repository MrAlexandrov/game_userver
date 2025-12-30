#pragma once

#include <string>
#include <userver/components/component_fwd.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/utils/fast_pimpl.hpp>

namespace game_userver {

class GetQuestionById final
    : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-get-question-by-id";

    GetQuestionById(
        const userver::components::ComponentConfig&,
        const userver::components::ComponentContext&
    );
    ~GetQuestionById() override;

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest&,
        userver::server::request::RequestContext&
    ) const override;

private:
    struct Impl;
    userver::utils::FastPimpl<Impl, 16, 8> impl_;
};

} // namespace game_userver
