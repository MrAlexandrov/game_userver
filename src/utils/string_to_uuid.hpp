#include <boost/uuid/uuid.hpp>
#include <string>

namespace Utils {

auto StringToUuid(const std::string& str) -> boost::uuids::uuid;

auto StringToBool(const std::string& str) -> bool;

} // namespace Utils
