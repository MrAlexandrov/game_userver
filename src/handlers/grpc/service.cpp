#include "service.hpp"

#include <models/models.pb.h> // proto model Pack

#include <boost/uuid/uuid_io.hpp> // for responce
#include <models/pack.hpp>        // cpp model Pack
#include <models/question.hpp>
#include <models/variant.hpp>
#include <utils/string_to_uuid.hpp>

#include "services/pack_service.hpp"
#include "services/question_service.hpp"
#include "services/variant_service.hpp"

namespace game_userver {

Service::Service(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : handlers::api::QuizServiceBase::Component(config, component_context),
      pack_service_(component_context.FindComponent<services::PackService>()),
      question_service_(
          component_context.FindComponent<services::QuestionService>()
      ),
      variant_service_(
          component_context.FindComponent<services::VariantService>()
      ) {}

Service::CreatePackResult
Service::CreatePack(CallContext&, handlers::api::CreatePackRequest&& request) {
    if (request.title().empty()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT, "Title cannot be empty"
        };
    }

    auto createdPackOpt = pack_service_.CreatePack(request.title());

    if (!createdPackOpt.has_value()) {
        return grpc::Status{
            grpc::StatusCode::INTERNAL, "Failed to create pack"
        };
    }
    auto createdPack = createdPackOpt.value();

    handlers::api::CreatePackResponse responce;
    auto mutualPack = responce.mutable_pack();
    mutualPack->set_id(boost::uuids::to_string(std::move(createdPack.id)));
    mutualPack->set_title(std::move(createdPack.title));
    return responce;
}

Service::GetPackByIdResult Service::GetPackById(
    CallContext&, handlers::api::GetPackByIdRequest&& request
) {
    auto pack_id = Utils::StringToUuid(request.id());
    if (pack_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.id()
        };
    }
    auto getPackByIdOpt = pack_service_.GetPackById(pack_id);

    if (!getPackByIdOpt.has_value()) {
        return grpc::Status{grpc::StatusCode::NOT_FOUND, "Pack not found"};
    }

    auto getPackById = getPackByIdOpt.value();

    handlers::api::GetPackByIdResponse responce;
    auto mutualPack = responce.mutable_pack();
    mutualPack->set_id(boost::uuids::to_string(std::move(getPackById.id)));
    mutualPack->set_title(std::move(getPackById.title));
    return responce;
}

Service::GetAllPacksResult Service::GetAllPacks(
    CallContext&, handlers::api::GetAllPacksRequest&& request
) {
    auto getAllPacks = pack_service_.GetAllPacks();

    handlers::api::GetAllPacksResponse responce;
    auto mutualPacks = responce.mutable_packs();
    for (auto&& pack : getAllPacks) {
        Models::Proto::Pack packResponse;
        packResponse.set_id(boost::uuids::to_string(pack.id));
        packResponse.set_title(std::move(pack.title));

        mutualPacks->Add(std::move(packResponse));
    }
    return responce;
}

Service::CreateQuestionResult Service::CreateQuestion(
    CallContext&, handlers::api::CreateQuestionRequest&& request
) {
    if (request.text().empty()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT, "Question text cannot be empty"
        };
    }

    auto pack_id = Utils::StringToUuid(request.pack_id());
    if (pack_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.pack_id()
        };
    }
    auto createdQuestionOpt = question_service_.CreateQuestion(
        pack_id, request.text(), request.image_url()
    );

    if (!createdQuestionOpt.has_value()) {
        return grpc::Status{
            grpc::StatusCode::INTERNAL, "Failed to create question"
        };
    }
    auto createdQuestion = createdQuestionOpt.value();

    handlers::api::CreateQuestionResponse response;
    auto mutableQuestion = response.mutable_question();
    mutableQuestion->set_id(
        boost::uuids::to_string(std::move(createdQuestion.id))
    );
    mutableQuestion->set_pack_id(
        boost::uuids::to_string(std::move(createdQuestion.pack_id))
    );
    mutableQuestion->set_text(std::move(createdQuestion.text));
    if (!createdQuestion.image_url.empty()) {
        mutableQuestion->set_image_url(createdQuestion.image_url);
    }

    return response;
}

Service::GetQuestionByIdResult Service::GetQuestionById(
    CallContext&, handlers::api::GetQuestionByIdRequest&& request
) {
    auto question_id = Utils::StringToUuid(request.id());
    if (question_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.id()
        };
    }
    auto questionOpt = question_service_.GetQuestionById(question_id);

    if (!questionOpt.has_value()) {
        return grpc::Status{grpc::StatusCode::NOT_FOUND, "Question not found"};
    }
    auto question = questionOpt.value();

    handlers::api::GetQuestionByIdResponse response;
    auto mutableQuestion = response.mutable_question();
    mutableQuestion->set_id(boost::uuids::to_string(std::move(question.id)));
    mutableQuestion->set_pack_id(
        boost::uuids::to_string(std::move(question.pack_id))
    );
    mutableQuestion->set_text(std::move(question.text));
    if (!question.image_url.empty()) {
        mutableQuestion->set_image_url(std::move(question.image_url));
    }

    return response;
}

Service::GetQuestionsByPackIdResult Service::GetQuestionsByPackId(
    CallContext&, handlers::api::GetQuestionsByPackIdRequest&& request
) {
    auto pack_id = Utils::StringToUuid(request.pack_id());
    if (pack_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.pack_id()
        };
    }
    auto questions = question_service_.GetQuestionsByPackId(pack_id);

    handlers::api::GetQuestionsByPackIdResponse response;
    auto mutableQuestions = response.mutable_questions();

    for (auto&& question : questions) {
        auto newQuestion = mutableQuestions->Add();
        newQuestion->set_id(boost::uuids::to_string(question.id));
        newQuestion->set_pack_id(boost::uuids::to_string(question.pack_id));
        newQuestion->set_text(std::move(question.text));
        if (!question.image_url.empty()) {
            newQuestion->set_image_url(std::move(question.image_url));
        }
    }

    return response;
}

Service::CreateVariantResult Service::CreateVariant(
    CallContext&, handlers::api::CreateVariantRequest&& request
) {
    if (request.text().empty()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT, "Variant text cannot be empty"
        };
    }

    auto question_id = Utils::StringToUuid(request.question_id());
    if (question_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.question_id()
        };
    }
    auto createdVariantOpt = variant_service_.CreateVariant(
        question_id, request.text(), request.is_correct()
    );

    if (!createdVariantOpt.has_value()) {
        return grpc::Status{
            grpc::StatusCode::INTERNAL, "Failed to create variant"
        };
    }
    auto createdVariant = createdVariantOpt.value();

    handlers::api::CreateVariantResponse response;
    auto mutableVariant = response.mutable_variant();
    mutableVariant->set_id(
        boost::uuids::to_string(std::move(createdVariant.id))
    );
    mutableVariant->set_question_id(
        boost::uuids::to_string(std::move(createdVariant.question_id))
    );
    mutableVariant->set_text(std::move(createdVariant.text));
    mutableVariant->set_is_correct(createdVariant.is_correct);

    return response;
}

Service::GetVariantByIdResult Service::GetVariantById(
    CallContext&, handlers::api::GetVariantByIdRequest&& request
) {
    auto variant_id = Utils::StringToUuid(request.id());
    if (variant_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.id()
        };
    }
    auto variantOpt = variant_service_.GetVariantById(variant_id);

    if (!variantOpt.has_value()) {
        return grpc::Status{grpc::StatusCode::NOT_FOUND, "Variant not found"};
    }
    auto variant = variantOpt.value();

    handlers::api::GetVariantByIdResponse response;
    auto mutableVariant = response.mutable_variant();
    mutableVariant->set_id(boost::uuids::to_string(std::move(variant.id)));
    mutableVariant->set_question_id(
        boost::uuids::to_string(std::move(variant.question_id))
    );
    mutableVariant->set_text(std::move(variant.text));
    mutableVariant->set_is_correct(variant.is_correct);

    return response;
}

Service::GetVariantsByQuestionIdResult Service::GetVariantsByQuestionId(
    CallContext&, handlers::api::GetVariantsByQuestionIdRequest&& request
) {
    auto question_id = Utils::StringToUuid(request.question_id());
    if (question_id.is_nil()) {
        return grpc::Status{
            grpc::StatusCode::INVALID_ARGUMENT,
            "Invalid UUID format: " + request.question_id()
        };
    }
    auto variants = variant_service_.GetVariantsByQuestionId(question_id);

    handlers::api::GetVariantsByQuestionIdResponse response;
    auto mutableVariants = response.mutable_variants();

    for (auto&& variant : variants) {
        auto newVariant = mutableVariants->Add();
        newVariant->set_id(boost::uuids::to_string(variant.id));
        newVariant->set_question_id(
            boost::uuids::to_string(variant.question_id)
        );
        newVariant->set_text(std::move(variant.text));
        newVariant->set_is_correct(variant.is_correct);
    }

    return response;
}

} // namespace game_userver
