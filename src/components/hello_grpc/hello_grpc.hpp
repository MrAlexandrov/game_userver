#pragma once

#include <handlers/hello_service.usrv.pb.hpp>
#include <userver/components/component.hpp>

namespace game_userver {

class HelloGrpc final : public handlers::api::HelloServiceBase::Component {
public:
    static constexpr std::string_view kName = "handler-hello-grpc";

    HelloGrpc(
        const userver::components::ComponentConfig&,
        const userver::components::ComponentContext&
    );

    auto
    SayHello(CallContext& /*context*/, handlers::api::HelloRequest&& /*request*/) -> SayHelloResult override;
};

} // namespace game_userver
