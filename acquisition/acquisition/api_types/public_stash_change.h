#pragma once

#include <acquisition/api_types/item.h>

#include <json_struct/json_struct.h>

#include <optional>
#include <string>
#include <vector>

namespace poe_api{

    // https://www.pathofexile.com/developer/docs/reference#type-PublicStashChange

    struct PublicStashChange {

        std::string id; // string a unique 64 digit hexadecimal string
        bool public_; // bool if false then optional properties will be null
        std::optional<std::string> accountName; // ? string
        std::optional<std::string> stash; // ? string the name of the stash
        std::optional<std::string> lastCharacterName; // ? string not included by default.Requires extra permissions
        std::string stashType; // string
        std::optional<std::string> league; // ? string the league's name
        std::vector<poe_api::Item> items; // array of Item

        JS_OBJECT(
            JS_MEMBER(id),
            JS_MEMBER_WITH_NAME(public_, "public"),
            JS_MEMBER(accountName),
            JS_MEMBER(stash),
            JS_MEMBER(lastCharacterName),
            JS_MEMBER(stashType),
            JS_MEMBER(league),
            JS_MEMBER(items));

    };

}
