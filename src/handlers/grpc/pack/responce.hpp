#pragma once

#include <handlers/cruds.pb.h>
#include <models/pack.hpp>

namespace game_userver {

handlers::api::CreatePackResponse CreateResponse(const NModels::Pack& pack);

} // namespace game_userver