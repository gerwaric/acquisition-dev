#pragma once

#include <json_struct/json_struct.h>

#include <optional>
#include <string>

namespace poe_api{

    // https://www.pathofexile.com/developer/docs/reference#type-LeagueRule
    
    struct LeagueRule {
        std::string                              id;             // string examples : Hardcore, NoParties(SSF)
        std::string                              name;           // string
        std::optional<std::string>                    description;    // ? string
        JS_OBJ(id, name, description);
    };

}
