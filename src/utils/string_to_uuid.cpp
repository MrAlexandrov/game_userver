#include "string_to_uuid.hpp"
#include <boost/uuid/string_generator.hpp>
#include <userver/utils/from_string.hpp>

namespace NUtils {

boost::uuids::uuid StringToUuid(const std::string& str) {
    const boost::uuids::string_generator generator;
    try {
        return generator(str);
    } catch (const std::exception& e) {
        return boost::uuids::uuid{};
    }
}

bool StringToBool(const std::string& str) {
    if (str.empty()) {
        return false;
    }

    std::string lower_str = str;
    std::transform(
        lower_str.begin(), lower_str.end(), lower_str.begin(),
        [](unsigned char c) {
            return std::tolower(c);
        }
    );

    if (lower_str == "true" || lower_str == "1" || lower_str == "yes" ||
        lower_str == "on") {
        return true;
    } else if (lower_str == "false" || lower_str == "0" || lower_str == "no" ||
               lower_str == "off") {
        return false;
    } else {
        throw std::invalid_argument("Cannot convert '" + str + "' to boolean");
    }
}

} // namespace NUtils
