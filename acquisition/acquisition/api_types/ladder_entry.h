#pragma once

#include <acquisition/api_types/account.h>

#include <json_struct/json_struct.h>

#include <optional>
#include <string>

namespace poe_api{

    // https://www.pathofexile.com/developer/docs/reference#type-LadderEntry

    struct LadderEntry {

        struct DelveInfo {
            std::optional<unsigned> default_; // ? uint
            std::optional<unsigned> solo; // ? uint
            JS_OBJECT(
                JS_MEMBER_WITH_NAME(default_, "default"),
                JS_MEMBER(solo));
        };

        struct Character {
            std::string id; // string a unique 64 digit hexadecimal string
            std::string name; // string
            unsigned level; // uint
            std::string class_; // string
            std::optional<unsigned> time; // ? uint time taken to complete the league objective in seconds
            std::optional<unsigned> score; // ? uint count of league objective completions
            // TBD progress; // ? object the values of this depend on the league objective
            std::optional<unsigned> experience; // ? uint
            std::optional<poe_api::LadderEntry::DelveInfo> depth; // ? object deepest Delve depth completed
            JS_OBJECT(
                JS_MEBMER(id),
                JS_MEMBER(name),
                JS_MEMBER(level),
                JS_MEMBER_WITH_NAME(class_, "class"),
                JS_MEMBER(time),
                JS_MEMBER(score),
                /* JS_MEMBER(progress), */
                JS_MEMBER(experience),
                JS_MEMBER(depth));
        };

        unsigned rank; // uint
        std::optional<bool> dead; // ? bool
        std::optional<bool> retired; // ? bool
        std::optional<bool> ineligible; // ? bool
        std::optional<bool> public_; // ? bool
        poe_api::LadderEntry::Character character; // object
        std::optional<poe_api::Account> account; // ? Account
        JS_OBJECT(
            JS_MEMBER(rank),
            JS_MEMBER(dead),
            JS_MEMBER(retired),
            JS_MEMBER(ineligible),
            JS_MEMBER_WITH_NAME(public_, "public"),
            JS_MEMBER(character),
            JS_MEMBER(account));

    };

}
