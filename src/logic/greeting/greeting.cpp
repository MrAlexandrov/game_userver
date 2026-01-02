#include "greeting.hpp"

#include <fmt/format.h>

#include <userver/utils/assert.hpp>

namespace game_userver {

auto SayHelloTo(std::string_view name, UserType type) -> std::string {
    if (name.empty()) {
        name = "unknown user";
    }

    switch (type) {
    case UserType::kFirstTime:
        return fmt::format("Hello, {}!\n", name);
    case UserType::kKnown:
        return fmt::format("Hi again, {}!\n", name);
    }

    UASSERT(false);
}

} // namespace game_userver
