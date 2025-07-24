#include <boost/uuid/uuid.hpp>
#include <string>

namespace NUtils {

boost::uuids::uuid GetUuidByString(const std::string& str);

} // namespace NUtils
