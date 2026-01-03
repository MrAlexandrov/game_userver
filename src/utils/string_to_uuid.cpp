#include "string_to_uuid.hpp"

#include <algorithm>
#include <boost/uuid/string_generator.hpp>
#include <userver/utils/from_string.hpp>

namespace Utils {

auto StringToUuid(const std::string& str) -> boost::uuids::uuid {
    const boost::uuids::string_generator generator;
    try {
        return generator(str);
    } catch (const std::exception& e) {
        return boost::uuids::uuid{};
    }
}

auto StringToBool(const std::string& str) -> bool {
    if (str.empty()) {
        return false;
    }

    std::string lower_str = str;
    std::ranges::transform(
        lower_str, lower_str.begin(),
        [](unsigned char symbol) -> int {
            return std::tolower(symbol);
        }
    );

    if (lower_str == "true" || lower_str == "1" || lower_str == "yes" ||
        lower_str == "on") {
        return true;
    }
    if (lower_str == "false" || lower_str == "0" || lower_str == "no" ||
        lower_str == "off") {
        return false;
    }
    throw std::invalid_argument("Cannot convert '" + str + "' to boolean");
}

} // namespace Utils
