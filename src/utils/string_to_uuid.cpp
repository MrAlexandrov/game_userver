#include <boost/uuid/string_generator.hpp>

namespace NUtils {

boost::uuids::uuid GetUuidByString(const std::string& str) {
    boost::uuids::string_generator generator;
    try {
        return generator(str);
    } catch (const std::exception& e) {
        return boost::uuids::uuid{};
    }
}

} // namespace NUtils
