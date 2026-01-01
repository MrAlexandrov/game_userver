#include <boost/uuid/uuid.hpp>
#include <string>

namespace Utils {

boost::uuids::uuid StringToUuid(const std::string& str);

bool StringToBool(const std::string& str);

} // namespace Utils
