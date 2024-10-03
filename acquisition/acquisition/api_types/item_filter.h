#pragma once

#include <json_struct/json_struct.h>

#include <acquisition/utils/json.h>

#include <QString>

#include <optional>

namespace poe_api {

    // https://www.pathofexile.com/developer/docs/reference#type-ItemFilter

    struct ItemFilter {

        struct Validity {
            bool valid; // bool
            std::optional<QString> version; // ? string game version
            std::optional<QString> validated; // ? string date time(ISO8601)
            JS_OBJ(valid, version, validated);
        };

        QString id; // string
        QString filter_name; // string
        QString realm; // string
        QString description; // string
        QString version; // string
        QString type; // string either Normal or Ruthless
        bool public_; // ? bool always true if present
        std::optional<QString> filter; // ? string not present when listing all filters
        std::optional<poe_api::ItemFilter::Validity> validation; // ? object not present when listing all filters
    
        JS_OBJECT(
            JS_MEMBER(id),
            JS_MEMBER(filter_name),
            JS_MEMBER(realm),
            JS_MEMBER(description),
            JS_MEMBER(version),
            JS_MEMBER(type),
            JS_MEMBER_WITH_NAME(public_, "public"),
            JS_MEMBER(filter),
            JS_MEMBER(validation));

    };

}
