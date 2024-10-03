#pragma once

#include <json_struct/json_struct.h>

#include <acquisition/utils/json.h>
#include <acquisition/api_types/display_mode.h>

#include <QString>

#include <optional>
#include <vector>

namespace poe_api {
    
    // https://www.pathofexile.com/developer/docs/reference#type-ItemProperty

    struct ItemProperty {
        QString name; // string
        // TBD std::vector<std::pair<QString,unsigned>> values; // array of array
        std::optional<poe_api::DisplayMode> displayMode; // ? uint as DisplayMode
        std::optional<double>progress; // ? double rounded to 2 decimal places
        std::optional<unsigned> type; // ? uint
        std::optional<QString> suffix; // ? string
        JS_OBJ(name, /* values, */ displayMode, progress, type, suffix);
    };

}
