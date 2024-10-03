#pragma once

#include <json_struct/json_struct.h>

#include <optional>
#include <string>

namespace poe_api{

    // https://www.pathofexile.com/developer/docs/reference#type-PvPMatch

    struct PvPMatch {

        std::string id; // string the match's name
        std::optional<std::string> realm; // ? string pc, xbox, or sony
        std::optional<std::string> startAt; // ? string date time(ISO8601)
        std::optional<std::string> endAt; // ? string date time(ISO8601)
        std::optional<std::string> url; // ? string a url link to a Path of Exile forum thread
        std::string description; // string
        bool glickoRatings; // bool
        bool pvp; // bool always true
        std::string style; // string Blitz, Swiss, or Arena
        std::optional<std::string> registerAt; // ? string date time(ISO8601)
        std::optional<bool>complete; // ? bool always true if present
        std::optional<bool>upcoming; // ? bool always true if present
        std::optional<bool> inProgress; // ? bool always true if present

        JS_OBJ(id, realm, startAt, endAt, url, description, glickoRatings, pvp, style, registerAt, complete, upcoming, inProgress);

    };

}
