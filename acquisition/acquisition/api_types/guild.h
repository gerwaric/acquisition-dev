#pragma once

#include <json_struct/json_struct.h>

#include <acquisition/utils/json.h>

#include <QString>

namespace poe_api {

    // https://www.pathofexile.com/developer/docs/reference#type-Guild

    struct Guild {
        unsigned id; // uint
        QString name; // string
        QString tag; // string
        JS_OBJ(id, name, tag);
    };

}
