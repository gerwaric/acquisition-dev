#pragma once

#include <json_struct/json_struct.h>

#include <acquisition/utils/json.h>
#include <acquisition/api_types/item.h>

#include <QString>

#include <optional>
#include <vector>

namespace poe_api{

    // https://www.pathofexile.com/developer/docs/reference#type-StashTab

    struct StashTab {

        struct Metadata {
            std::optional<bool> public_; // ? bool always true if present
            std::optional<bool> folder; // ? bool always true if present
            std::optional<QString> colour; // ? string 6 digit hex colour (NOTE: might be only 2 or 4 characters).
            JS_OBJECT(
                JS_MEMBER_WITH_NAME(public_, "public"),
                JS_MEMBER(folder),
                JS_MEMBER(colour));
        };

        QString id; // string a 10 digit hexadecimal string
        std::optional<QString> parent; // ? string a 10 digit hexadecimal string
        QString name; // string
        QString type; // string
        std::optional<unsigned> index; // ? uint
        poe_api::StashTab::Metadata metadata; // metadata object
        std::optional<std::vector<poe_api::StashTab>> children; // ? array of StashTab
        std::optional<std::vector<poe_api::Item>> items; // ? array of Item
        JS_OBJ(id, parent, name, type, index, metadata, children, items);
    };

};
