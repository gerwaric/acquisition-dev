#pragma once

#include <json_struct/json_struct.h>

#include <acquisition/utils/json.h>

namespace poe_api {

    // https://www.pathofexile.com/developer/docs/reference#type-DisplayMode

    enum class DisplayMode : unsigned {
        NameFirst = 0, // Name should be followed by values
        ValuesFirst = 1, // Values should be followed by name
        ProgressBar = 2, // Progress bar
        InsertedValues = 3, // Values should be inserted into the string by index
        Separator = 4 // Separator
    };

}

JS_ENUM_NAMESPACE_DECLARE_VALUE_PARSER(poe_api, DisplayMode);
