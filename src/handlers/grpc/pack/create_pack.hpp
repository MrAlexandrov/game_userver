#pragma once

#include <userver/components/component.hpp>

#include <handlers/cruds_service.usrv.pb.hpp>
#include <userver/storages/postgres/postgres_fwd.hpp>

namespace game_userver {

class CreatePackGrpc final : public handlers::api::QuizServiceBase::Component {
public:
    static constexpr std::string_view kName = "handler-create-pack-grpc";

    CreatePackGrpc(
          const userver::components::ComponentConfig&
        , const userver::components::ComponentContext&
    );

    CreatePackResult CreatePack(
          CallContext&
        , handlers::api::CreatePackRequest&&
    ) override;

private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};

} // namespace game_userver
