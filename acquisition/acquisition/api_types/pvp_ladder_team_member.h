#pragma once

#include <acquisition/api_types/account.h>

#include <json_struct/json_struct.h>

#include <optional>
#include <string>

namespace poe_api{

    // https://www.pathofexile.com/developer/docs/reference#type-PvPLadderTeamMember

    struct PvPLadderTeamMember {

        struct Character {
            std::string id; // string a unique 64 digit hexadecimal string
            std::string name; // string
            unsigned level; // uint
            std::string class_; // string
            std::optional<std::string> league; // ? string
            std::optional<unsigned> score; // ? uint count of league objective completions
            
            JS_OBJECT(
                JS_MEMBER(id),
                JS_MEMBER(name),
                JS_MEMBER(level),
                JS_MEMBER_WITH_NAME(class_, "class"),
                JS_MEMBER(league),
                JS_MEMBER(score));
        };

        poe_api::Account account; // Account
        poe_api::PvPLadderTeamMember::Character character; // object
        std::optional<bool> public_; // ? bool always true if present
        
        JS_OBJECT(
            JS_MEMBER(account),
            JS_MEMBER(character),
            JS_MEMBER_WITH_NAME(public_, "public"));
    };

}
