#pragma once

#include <json_struct/json_struct.h>

#include <acquisition/utils/json.h>

namespace poe_api {

    // https://www.pathofexile.com/developer/docs/reference#type-FrameType

    enum class FrameType : unsigned {
        Normal = 0,
        Magic = 1,
        Rare = 2,
        Unique = 3,
        Gem = 4,
        Currency = 5,
        DivinationCard = 6,
        Quest = 7,
        Prophecy = 8,
        Foil = 9,
        SupporterFoil = 10,
        Necropolis = 11
    };

}

JS_ENUM_NAMESPACE_DECLARE_VALUE_PARSER(poe_api, FrameType);
