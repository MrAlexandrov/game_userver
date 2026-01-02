#pragma once

#include <userver/components/component_list.hpp>

namespace game_userver::question {

auto GetQuestionHandlersComponentList() -> userver::components::ComponentList;

} // namespace game_userver::question
