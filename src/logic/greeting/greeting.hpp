#pragma once

#include <string>
#include <string_view>

namespace game_userver {

enum class UserType {
    kFirstTime,
    kKnown
};

auto SayHelloTo(std::string_view name, UserType type) -> std::string;

} // namespace game_userver
