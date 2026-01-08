#include "questions_and_variants.hpp"

#include <sql_queries/sql_queries.hpp>
#include <userver/storages/postgres/cluster_types.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/io/io_fwd.hpp>

#include "models/question.hpp"
#include "models/variant.hpp"

#include <map>

namespace NStorage {

using userver::storages::postgres::ClusterPtr;
using namespace sql_queries::sql;
using userver::storages::postgres::ClusterHostType::kMaster;
using userver::storages::postgres::ClusterHostType::kSlave;

auto GetQuestionsAndVariantsByPackId(
    ClusterPtr pg_cluster_, const boost::uuids::uuid& pack_id
) -> std::vector<std::pair<Models::Question, std::vector<Models::Variant>>> {
    const auto result =
        pg_cluster_->Execute(kSlave, kGetQuestionsAndVariantsByPackId, pack_id);

    // Group variants by question_id
    std::map<boost::uuids::uuid, Models::Question> questions_map;
    std::map<boost::uuids::uuid, std::vector<Models::Variant>> variants_map;

    for (const auto& row : result) {
        // Try to get question data
        {
            const auto question_id =
                row["question_id"].As<boost::uuids::uuid>();
            Models::Question question = {
                .id = question_id,
                .pack_id = row["question_pack_id"].As<boost::uuids::uuid>(),
                .text = row["question_text"].As<std::string>(),
                .image_url = row["question_image_url"].As<std::string>(),
            };
            questions_map[question_id] = std::move(question);
        }

        // Try to get variant data (only if variant exists - not NULL from LEFT
        // JOIN)
        if (!row["variant_id"].IsNull()) {
            const auto question_id =
                row["variant_question_id"].As<boost::uuids::uuid>();
            Models::Variant variant = {
                .id = row["variant_id"].As<boost::uuids::uuid>(),
                .question_id = question_id,
                .text = row["variant_text"].As<std::string>(),
                .is_correct = row["variant_is_correct"].As<bool>(),
            };
            variants_map[question_id].push_back(std::move(variant));
        }
    }

    // Combine questions and their variants
    std::vector<std::pair<Models::Question, std::vector<Models::Variant>>>
        result_pairs;
    for (const auto& [question_id, question] : questions_map) {

        if (const auto variants_it = variants_map.find(question_id);
            variants_it != variants_map.end()) {
            result_pairs.emplace_back(question, variants_it->second);
        } else {
            result_pairs.emplace_back(question, std::vector<Models::Variant>{});
        }
    }

    return result_pairs;
}

} // namespace NStorage
