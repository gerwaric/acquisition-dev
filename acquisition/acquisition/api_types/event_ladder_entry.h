#pragma once

#include <json_struct/json_struct.h>

#include <optional>
#include <string>

namespace poe_api{

    // https://www.pathofexile.com/developer/docs/reference#type-EventLadderEntry

    struct EventLadderEntry {

        struct PrivateLeague {
            std::string name; // string
            std::string url; // string a url link to a Path of Exile Private League
            JS_OBJ(name, url);
        };

        unsigned rank; // uint
        std::optional<bool> ineligible; // ? bool
        std::optional<unsigned> time; // ? uint time taken to complete the league objective in seconds
        poe_api::EventLadderEntry::PrivateLeague private_league; // object
        JS_OBJ(rank, ineligible, time, private_league);
    };

}
