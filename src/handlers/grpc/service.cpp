#include "service.hpp"

#include <models/models.pb.h> // proto model Pack

#include <boost/uuid/uuid_io.hpp> // for responce
#include <expected>
#include <models/pack.hpp> // cpp model Pack
#include <models/question.hpp>
#include <models/variant.hpp>
#include <userver/storages/postgres/component.hpp>
#include <utils/string_to_uuid.hpp>

#include "utils/pack.hpp"
#include "utils/question.hpp"
#include "utils/variant.hpp"

namespace game_userver {

namespace {

void FillMutablePack(::Models::Proto::Pack* mutablePack, const Models::Pack& pack) {
    mutablePack->set_id(boost::uuids::to_string(pack.id));
    mutablePack->set_title(pack.title);
}

void FillMutableQuestion(::Models::Proto::Question* mutableQuestion, const Models::Question& question) {
    mutableQuestion->set_id(boost::uuids::to_string(question.id));
    mutableQuestion->set_pack_id(
        boost::uuids::to_string(question.pack_id)
    );
    mutableQuestion->set_text(question.text);
    if (!question.image_url.empty()) {
        mutableQuestion->set_image_url(question.image_url);
    }
    mutableQuestion->set_question_type(
        Models::ToString(question.question_type)
    );
}

void FillMutableVariant(::Models::Proto::Variant* mutableVariant, const Models::Variant& variant) {
    mutableVariant->set_id(boost::uuids::to_string(variant.id));
    mutableVariant->set_question_id(
        boost::uuids::to_string(variant.question_id)
    );
    mutableVariant->set_text(variant.text);
    mutableVariant->set_is_correct(variant.is_correct);
}

} // anonymous namespace

Service::Service(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : handlers::api::QuizServiceBase::Component(config, component_context),
      storage_(config, component_context) {}

auto Service::CreatePack(
    CallContext& /*context*/, handlers::api::CreatePackRequest&& request
) -> Service::CreatePackResult {
    auto pack = Utils::GetPackFromRequest(std::move(request));
    if (!pack.has_value()) {
        return pack.error();
    }
    const auto created_pack_opt = storage_.CreatePack(pack.value());

    if (!created_pack_opt.has_value()) {
        return grpc::Status{
            grpc::StatusCode::INTERNAL, "Failed to create pack"
        };
    }
    const auto& created_pack = created_pack_opt.value();

    handlers::api::CreatePackResponse responce;
    FillMutablePack(responce.mutable_pack(), created_pack);
    return responce;
}

auto Service::GetPackById(
    CallContext& /*context*/, handlers::api::GetPackByIdRequest&& request
) -> Service::GetPackByIdResult {
    const auto pack_id = Utils::StringToUuid(request.id());
    if (pack_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.id()
        };
    }
    const auto pack_opt = storage_.GetPackById(pack_id);

    if (!pack_opt.has_value()) {
        return grpc::Status{grpc::StatusCode::NOT_FOUND, "Pack not found"};
    }

    const auto& pack = pack_opt.value();

    handlers::api::GetPackByIdResponse responce;
    FillMutablePack(responce.mutable_pack(), pack);
    return responce;
}

auto Service::GetAllPacks(
    CallContext& /*context*/, handlers::api::GetAllPacksRequest&& request
) -> Service::GetAllPacksResult {
    auto getAllPacks = storage_.GetAllPacks();

    handlers::api::GetAllPacksResponse responce;
    auto* mutablePacks = responce.mutable_packs();
    for (auto&& pack : getAllPacks) {
        Models::Proto::Pack packResponse;
        FillMutablePack(&packResponse, pack);

        mutablePacks->Add(std::move(packResponse));
    }
    return responce;
}

auto Service::CreateQuestion(
    CallContext& /*context*/, handlers::api::CreateQuestionRequest&& request
) -> Service::CreateQuestionResult {
    auto question = Utils::GetQuestionFromRequest(std::move(request));
    if (!question.has_value()) {
        return question.error();
    }

    const auto createdQuestionOpt =
        storage_.CreateQuestion(std::move(question.value()));

    if (!createdQuestionOpt.has_value()) {
        return grpc::Status{
            grpc::StatusCode::INTERNAL, "Failed to create question"
        };
    }
    const auto& createdQuestion = createdQuestionOpt.value();

    handlers::api::CreateQuestionResponse response;
    FillMutableQuestion(response.mutable_question(), createdQuestion);
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
    auto questionOpt = storage_.GetQuestionById(question_id);

    if (!questionOpt.has_value()) {
        return grpc::Status{grpc::StatusCode::NOT_FOUND, "Question not found"};
    }
    const auto& question = questionOpt.value();

    handlers::api::GetQuestionByIdResponse response;
    FillMutableQuestion(response.mutable_question(), question);
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
    auto questions = storage_.GetQuestionsByPackId(pack_id);

    handlers::api::GetQuestionsByPackIdResponse response;
    auto* mutableQuestions = response.mutable_questions();

    for (auto&& question : questions) {
        auto* newQuestion = mutableQuestions->Add();
        FillMutableQuestion(newQuestion, question);
    }

    return response;
}

auto Service::CreateVariant(
    CallContext& /*context*/, handlers::api::CreateVariantRequest&& request
) -> Service::CreateVariantResult {
    auto variant = Utils::GetVariantFromRequest(std::move(request));
    if (!variant.has_value()) {
        return variant.error();
    }
    auto createdVariantOpt = storage_.CreateVariant(variant.value());

    if (!createdVariantOpt.has_value()) {
        return grpc::Status{
            grpc::StatusCode::INTERNAL, "Failed to create variant"
        };
    }
    const auto& createdVariant = createdVariantOpt.value();

    handlers::api::CreateVariantResponse response;
    FillMutableVariant(response.mutable_variant(), createdVariant);
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
    auto variantOpt = storage_.GetVariantById(variant_id);

    if (!variantOpt.has_value()) {
        return grpc::Status{grpc::StatusCode::NOT_FOUND, "Variant not found"};
    }
    const auto& variant = variantOpt.value();

    handlers::api::GetVariantByIdResponse response;
    FillMutableVariant(response.mutable_variant(), variant);
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
    auto variants = storage_.GetVariantsByQuestionId(question_id);

    handlers::api::GetVariantsByQuestionIdResponse response;
    auto* mutableVariants = response.mutable_variants();

    for (auto&& variant : variants) {
        auto* newVariant = mutableVariants->Add();
        FillMutableVariant(newVariant, variant);
    }

    return response;
}

} // namespace game_userver
