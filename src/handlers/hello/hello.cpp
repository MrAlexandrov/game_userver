#include "hello.hpp"

#include "logic/greeting/greeting.hpp"

namespace game_userver {

auto Hello::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&
    /*context*/
) const -> std::string {
    return SayHelloTo(request.GetArg("name"), UserType::kFirstTime);
}

} // namespace game_userver
