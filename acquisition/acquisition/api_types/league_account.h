#pragma once

#include <json_struct/json_struct.h>

#include <acquisition/utils/json.h>

#include <QString>

#include <vector>

namespace poe_api{

    // https://www.pathofexile.com/developer/docs/reference#type-LeagueAccount           

    struct LeagueAccount {

        struct AtlasPassiveTree {
            QString name; // string
            std::vector<unsigned> hashes; // array of uint
            JS_OBJ(name, hashes);
        };

        std::vector<poe_api::LeagueAccount::AtlasPassiveTree> atlas_passive_trees; // array of object
        JS_OBJ(atlas_passive_trees)

    };

}
