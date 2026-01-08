#include "create_pack_from_yaml.hpp"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <sql_queries/sql_queries.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/yaml.hpp>
#include <userver/logging/log.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

#include "models/pack.hpp"
#include "models/question.hpp"
#include "models/variant.hpp"
#include "storage/packs.hpp"
#include "storage/questions.hpp"
#include "storage/variants.hpp"
#include "utils/constants.hpp"

namespace game_userver {

struct CreatePackFromYaml::Impl {
    userver::storages::postgres::ClusterPtr pg_cluster;

    explicit Impl(const userver::components::ComponentContext& context)
        : pg_cluster(context
                         .FindComponent<userver::components::Postgres>(
                             Constants::kDatabaseName
                         )
                         .GetCluster()) {}
};

CreatePackFromYaml::CreatePackFromYaml(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& component_context
)
    : HttpHandlerBase(config, component_context), impl_(component_context) {}

CreatePackFromYaml::~CreatePackFromYaml() = default;

auto CreatePackFromYaml::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
    /*context*/
) const -> std::string {
    using userver::logging::Level::kDebug;
    using userver::logging::Level::kError;
    using userver::logging::Level::kInfo;

    // Get YAML content from request body
    const auto& request_body = request.RequestBody();
    if (request_body.empty()) {
        request.GetHttpResponse().SetStatus(
            userver::server::http::HttpStatus::kBadRequest
        );
        return R"({"error": "Request body is empty"})";
    }

    try {
        // Parse YAML
        auto yaml = userver::formats::yaml::FromString(request_body);

        // Extract pack title
        auto title = yaml["title"].As<std::string>();
        if (title.empty()) {
            request.GetHttpResponse().SetStatus(
                userver::server::http::HttpStatus::kBadRequest
            );
            return R"({"error": "Pack title is required"})";
        }

        LOG(kInfo) << "Creating pack from YAML: " << title;

        // Create pack
        Models::Pack pack;
        pack.title = title;

        const auto createdPackOpt =
            NStorage::CreatePack(impl_->pg_cluster, pack);
        if (!createdPackOpt) {
            request.GetHttpResponse().SetStatus(
                userver::server::http::HttpStatus::kInternalServerError
            );
            return R"({"error": "Failed to create pack"})";
        }

        const auto& createdPack = createdPackOpt.value();
        LOG(kDebug) << "Created pack with ID: "
                    << boost::uuids::to_string(createdPack.id);

        // Process questions
        const auto& questions_yaml = yaml["questions"];
        if (!questions_yaml.IsMissing() && questions_yaml.IsArray()) {
            for (const auto& question_yaml : questions_yaml) {
                // Extract question text
                auto question_text = question_yaml["text"].As<std::string>();
                if (question_text.empty()) {
                    LOG(kError) << "Skipping question with empty text";
                    continue;
                }

                // Create question
                Models::Question question;
                question.pack_id = createdPack.id;
                question.text = question_text;
                question.image_url =
                    question_yaml["image_url"].As<std::string>("");

                const auto createdQuestionOpt = NStorage::CreateQuestion(
                    impl_->pg_cluster, std::move(question)
                );
                if (!createdQuestionOpt) {
                    LOG(kError)
                        << "Failed to create question: " << question_text;
                    continue;
                }

                const auto& createdQuestion = createdQuestionOpt.value();
                LOG(kDebug) << "Created question with ID: "
                            << boost::uuids::to_string(createdQuestion.id);

                // Process variants
                const auto& variants_yaml = question_yaml["variants"];
                if (!variants_yaml.IsMissing() && variants_yaml.IsArray()) {
                    for (const auto& variant_yaml : variants_yaml) {
                        // Extract variant text
                        auto variant_text =
                            variant_yaml["text"].As<std::string>();
                        if (variant_text.empty()) {
                            LOG(kError) << "Skipping variant with empty text";
                            continue;
                        }

                        // Create variant
                        Models::Variant variant;
                        variant.question_id = createdQuestion.id;
                        variant.text = variant_text;
                        variant.is_correct =
                            variant_yaml["is_correct"].As<bool>(false);

                        const auto createdVariantOpt =
                            NStorage::CreateVariant(impl_->pg_cluster, variant);
                        if (!createdVariantOpt) {
                            LOG(kError)
                                << "Failed to create variant: " << variant_text;
                            continue;
                        }

                        LOG(kDebug) << "Created variant with ID: "
                                    << boost::uuids::to_string(
                                           createdVariantOpt.value().id
                                       );
                    }
                }
            }
        }

        // Return created pack info
        userver::formats::json::ValueBuilder response;
        response["id"] = boost::uuids::to_string(createdPack.id);
        response["title"] = createdPack.title;
        response["message"] = "Pack created successfully from YAML";

        return userver::formats::json::ToPrettyString(response.ExtractValue());

    } catch (const userver::formats::yaml::Exception& e) {
        LOG(kError) << "YAML parsing error: " << e.what();
        request.GetHttpResponse().SetStatus(
            userver::server::http::HttpStatus::kBadRequest
        );
        return R"({"error": "Invalid YAML format"})";
    } catch (const std::exception& e) {
        LOG(kError) << "Error creating pack from YAML: " << e.what();
        request.GetHttpResponse().SetStatus(
            userver::server::http::HttpStatus::kInternalServerError
        );
        return R"({"error": "Internal server error"})";
    }
}

} // namespace game_userver
