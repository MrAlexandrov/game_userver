#include "service.hpp"

#include <models/models.pb.h> // proto model Pack

#include <boost/uuid/uuid_io.hpp> // for responce
#include <expected>
#include <models/pack.hpp> // cpp model Pack
#include <models/question.hpp>
#include <models/variant.hpp>
#include <userver/storages/postgres/component.hpp>
#include <utils/string_to_uuid.hpp>

#include "storage/packs.hpp" // for db request CreatePack
#include "storage/questions.hpp"
#include "storage/variants.hpp"
#include "utils/constants.hpp"
#include "utils/pack.hpp"
#include "utils/question.hpp"
#include "utils/variant.hpp"

namespace game_userver {

Service::Service(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : handlers::api::QuizServiceBase::Component(config, component_context),
      pg_cluster_(component_context
                      .FindComponent<userver::components::Postgres>(
                          Constants::kDatabaseName
                      )
                      .GetCluster()) {}

auto Service::CreatePack(
    CallContext&, handlers::api::CreatePackRequest&& request
) -> Service::CreatePackResult {
    auto pack_data = Utils::GetPackDataFromRequest(std::move(request));
    if (!pack_data.has_value()) {
        return pack_data.error();
    }

    const auto createdPackOpt =
        NStorage::CreatePack(pg_cluster_, pack_data.value());

    if (!createdPackOpt.has_value()) {
        return grpc::Status{
            grpc::StatusCode::INTERNAL, "Failed to create pack"
        };
    }
    auto createdPack = createdPackOpt.value();

    handlers::api::CreatePackResponse responce;
    auto* mutualPack = responce.mutable_pack();
    mutualPack->set_id(boost::uuids::to_string(createdPack.id));
    mutualPack->set_title(std::move(createdPack.data.title));
    return responce;
}

auto Service::GetPackById(
    CallContext& /*context*/, handlers::api::GetPackByIdRequest&& request
) -> Service::GetPackByIdResult {
    auto pack_id = Utils::StringToUuid(request.id());
    if (pack_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.id()
        };
    }
    auto getPackByIdOpt = NStorage::GetPackById(pg_cluster_, pack_id);

    if (!getPackByIdOpt.has_value()) {
        return grpc::Status{grpc::StatusCode::NOT_FOUND, "Pack not found"};
    }

    auto getPackById = getPackByIdOpt.value();

    handlers::api::GetPackByIdResponse responce;
    auto* mutualPack = responce.mutable_pack();
    mutualPack->set_id(boost::uuids::to_string(getPackById.id));
    mutualPack->set_title(std::move(getPackById.data.title));
    return responce;
}

auto Service::GetAllPacks(
    CallContext& /*context*/, handlers::api::GetAllPacksRequest&& request
) -> Service::GetAllPacksResult {
    auto getAllPacks = NStorage::GetAllPacks(pg_cluster_);

    handlers::api::GetAllPacksResponse responce;
    auto* mutualPacks = responce.mutable_packs();
    for (auto&& pack : getAllPacks) {
        Models::Proto::Pack packResponse;
        packResponse.set_id(boost::uuids::to_string(pack.id));
        packResponse.set_title(std::move(pack.data.title));

        mutualPacks->Add(std::move(packResponse));
    }
    return responce;
}

auto Service::CreateQuestion(
    CallContext& /*context*/, handlers::api::CreateQuestionRequest&& request
) -> Service::CreateQuestionResult {
    auto question_data = Utils::GetQuestionDataFromRequest(std::move(request));
    if (!question_data.has_value()) {
        return question_data.error();
    }

    const auto createdQuestionOpt =
        NStorage::CreateQuestion(pg_cluster_, std::move(question_data.value()));

    if (!createdQuestionOpt.has_value()) {
        return grpc::Status{
            grpc::StatusCode::INTERNAL, "Failed to create question"
        };
    }
    auto createdQuestion = createdQuestionOpt.value();

    handlers::api::CreateQuestionResponse response;
    auto* mutableQuestion = response.mutable_question();
    mutableQuestion->set_id(boost::uuids::to_string(createdQuestion.id));
    mutableQuestion->set_pack_id(
        boost::uuids::to_string(createdQuestion.data.pack_id)
    );
    mutableQuestion->set_text(std::move(createdQuestion.data.text));
    if (!createdQuestion.data.image_url.empty()) {
        mutableQuestion->set_image_url(createdQuestion.data.image_url);
    }

    return response;
}

auto Service::GetQuestionById(
    CallContext& /*context*/, handlers::api::GetQuestionByIdRequest&& request
) -> Service::GetQuestionByIdResult {
    auto question_id = Utils::StringToUuid(request.id());
    if (question_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.id()
        };
    }
    auto questionOpt = NStorage::GetQuestionById(pg_cluster_, question_id);

    if (!questionOpt.has_value()) {
        return grpc::Status{grpc::StatusCode::NOT_FOUND, "Question not found"};
    }
    auto question = questionOpt.value();

    handlers::api::GetQuestionByIdResponse response;
    auto* mutableQuestion = response.mutable_question();
    mutableQuestion->set_id(boost::uuids::to_string(question.id));
    mutableQuestion->set_pack_id(
        boost::uuids::to_string(question.data.pack_id)
    );
    mutableQuestion->set_text(std::move(question.data.text));
    if (!question.data.image_url.empty()) {
        mutableQuestion->set_image_url(std::move(question.data.image_url));
    }

    return response;
}

auto Service::GetQuestionsByPackId(
    CallContext& /*context*/,
    handlers::api::GetQuestionsByPackIdRequest&& request
) -> Service::GetQuestionsByPackIdResult {
    auto pack_id = Utils::StringToUuid(request.pack_id());
    if (pack_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.pack_id()
        };
    }
    auto questions = NStorage::GetQuestionsByPackId(pg_cluster_, pack_id);

    handlers::api::GetQuestionsByPackIdResponse response;
    auto* mutableQuestions = response.mutable_questions();

    for (auto&& question : questions) {
        auto* newQuestion = mutableQuestions->Add();
        newQuestion->set_id(boost::uuids::to_string(question.id));
        newQuestion->set_pack_id(
            boost::uuids::to_string(question.data.pack_id)
        );
        newQuestion->set_text(std::move(question.data.text));
        if (!question.data.image_url.empty()) {
            newQuestion->set_image_url(std::move(question.data.image_url));
        }
    }

    return response;
}

auto Service::CreateVariant(
    CallContext& /*context*/, handlers::api::CreateVariantRequest&& request
) -> Service::CreateVariantResult {
    auto variant_data = Utils::GetVariantDataFromRequest(std::move(request));
    if (!variant_data.has_value()) {
        return variant_data.error();
    }
    auto createdVariantOpt =
        NStorage::CreateVariant(pg_cluster_, variant_data.value());

    if (!createdVariantOpt.has_value()) {
        return grpc::Status{
            grpc::StatusCode::INTERNAL, "Failed to create variant"
        };
    }
    auto createdVariant = createdVariantOpt.value();

    handlers::api::CreateVariantResponse response;
    auto* mutableVariant = response.mutable_variant();
    mutableVariant->set_id(boost::uuids::to_string(createdVariant.id));
    mutableVariant->set_question_id(
        boost::uuids::to_string(createdVariant.data.question_id)
    );
    mutableVariant->set_text(std::move(createdVariant.data.text));
    mutableVariant->set_is_correct(createdVariant.data.is_correct);

    return response;
}

auto Service::GetVariantById(
    CallContext& /*context*/, handlers::api::GetVariantByIdRequest&& request
) -> Service::GetVariantByIdResult {
    auto variant_id = Utils::StringToUuid(request.id());
    if (variant_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.id()
        };
    }
    auto variantOpt = NStorage::GetVariantById(pg_cluster_, variant_id);

    if (!variantOpt.has_value()) {
        return grpc::Status{grpc::StatusCode::NOT_FOUND, "Variant not found"};
    }
    auto variant = variantOpt.value();

    handlers::api::GetVariantByIdResponse response;
    auto* mutableVariant = response.mutable_variant();
    mutableVariant->set_id(boost::uuids::to_string(variant.id));
    mutableVariant->set_question_id(
        boost::uuids::to_string(variant.data.question_id)
    );
    mutableVariant->set_text(std::move(variant.data.text));
    mutableVariant->set_is_correct(variant.data.is_correct);

    return response;
}

auto Service::GetVariantsByQuestionId(
    CallContext& /*context*/,
    handlers::api::GetVariantsByQuestionIdRequest&& request
) -> Service::GetVariantsByQuestionIdResult {
    auto question_id = Utils::StringToUuid(request.question_id());
    if (question_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.question_id()
        };
    }
    auto variants = NStorage::GetVariantsByQuestionId(pg_cluster_, question_id);

    handlers::api::GetVariantsByQuestionIdResponse response;
    auto* mutableVariants = response.mutable_variants();

    for (auto&& variant : variants) {
        auto* newVariant = mutableVariants->Add();
        newVariant->set_id(boost::uuids::to_string(variant.id));
        newVariant->set_question_id(
            boost::uuids::to_string(variant.data.question_id)
        );
        newVariant->set_text(std::move(variant.data.text));
        newVariant->set_is_correct(variant.data.is_correct);
    }

    return response;
}

} // namespace game_userver
