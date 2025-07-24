#include "hello.hpp"

#include "logic/greeting/greeting.hpp"

namespace game_userver {

std::string Hello::HandleRequestThrow(
      const userver::server::http::HttpRequest& request
    , userver::server::request::RequestContext&
) const {
    return SayHelloTo(request.GetArg("name"), UserType::kFirstTime);
}

} // namespace game_userver
