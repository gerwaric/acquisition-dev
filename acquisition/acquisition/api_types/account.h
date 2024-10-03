#pragma once

#include <json_struct/json_struct.h>

#include <acquisition/utils/json.h>
#include <acquisition/api_types/guild.h>

#include <QString>

#include <optional>

namespace poe_api {

    // https://www.pathofexile.com/developer/docs/reference#type-Account

    struct Account {

        struct Challenges {
            QString set; // string the challenge set
            unsigned completed; // uint
            unsigned max; // uint
            JS_OBJ(set, completed, max);
        };

        struct TwitchStream {
            QString name; // string
            QString image; // string
            QString status; // string
            JS_OBJ(name, image, status);
        };

        struct Twitch {
            QString name; // string
            std::optional<poe_api::Account::TwitchStream> stream; // ? object
            JS_OBJ(name, stream);
        };

        QString name; // string
        std::optional<QString> realm; // ? string pc, xbox, or sony
        std::optional<poe_api::Guild> guild; // ? Guild
        std::optional<poe_api::Account::Challenges> challenges; // ? object
        std::optional<poe_api::Account::Twitch> twitch; // ? object
        JS_OBJ(name, realm, guild, challenges, twitch);
    };
}
