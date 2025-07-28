#include "responce.hpp"

#include <boost/uuid/uuid_io.hpp>

namespace game_userver {

handlers::api::CreatePackResponse CreateResponse(const NModels::Pack& pack) {
    handlers::api::CreatePackResponse responce;
    auto mutualPack = responce.mutable_pack();
    mutualPack->set_id(boost::uuids::to_string(pack.id));
    mutualPack->set_title(pack.title);
    return responce;
}

} // namespace game_userver
