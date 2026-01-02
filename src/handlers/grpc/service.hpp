#pragma once

#include <handlers/cruds.pb.h> // for responce

#include <handlers/cruds_service.usrv.pb.hpp>
#include <models/pack.hpp> // for responce
#include <userver/components/component.hpp>
#include <userver/storages/postgres/postgres_fwd.hpp>

namespace game_userver {

class Service final : public handlers::api::QuizServiceBase::Component {
public:
    static constexpr std::string_view kName = "handler-service-grpc";

    Service(
        const userver::components::ComponentConfig&,
        const userver::components::ComponentContext&
    );

    auto CreatePack(
        CallContext& /*context*/, handlers::api::CreatePackRequest&& /*request*/
    ) -> CreatePackResult override;

    auto GetPackById(
        CallContext& /*context*/,
        handlers::api::GetPackByIdRequest&& /*request*/
    ) -> GetPackByIdResult override;

    auto GetAllPacks(
        CallContext& /*context*/,
        handlers::api::GetAllPacksRequest&& /*request*/
    ) -> GetAllPacksResult override;

    auto CreateQuestion(
        CallContext& /*context*/, handlers::api::CreateQuestionRequest&&
        /*request*/
    ) -> CreateQuestionResult override;

    auto GetQuestionById(
        CallContext& /*context*/, handlers::api::GetQuestionByIdRequest&&
        /*request*/
    ) -> GetQuestionByIdResult override;

    auto GetQuestionsByPackId(
        CallContext& /*context*/, handlers::api::GetQuestionsByPackIdRequest&&
        /*request*/
    ) -> GetQuestionsByPackIdResult override;

    // === Variant operations ===
    auto CreateVariant(
        CallContext& /*context*/,
        handlers::api::CreateVariantRequest&& /*request*/
    ) -> CreateVariantResult override;

    auto GetVariantById(
        CallContext& /*context*/, handlers::api::GetVariantByIdRequest&&
        /*request*/
    ) -> GetVariantByIdResult override;

    auto GetVariantsByQuestionId(
        CallContext& /*context*/,
        handlers::api::GetVariantsByQuestionIdRequest&&
        /*request*/
    ) -> GetVariantsByQuestionIdResult override;

private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};

} // namespace game_userver
