#pragma once

#include <json_struct/json_struct.h>

#include <acquisition/utils/json.h>
#include <acquisition/api_types/passive_group.h>
#include <acquisition/api_types/passive_node.h>

#include <QString>

#include <map>
#include <optional>

namespace poe_api {
    
    // https://www.pathofexile.com/developer/docs/reference#type-ItemJewelData

    struct ItemJewelData {

        struct Subgraph {
            std::map<QString, poe_api::PassiveGroup> groups; // dictionary of PassiveGroup the key is the string value of the group id
            std::map<QString, poe_api::PassiveNode> nodes; // dictionary of PassiveNode the key is the string value of the node identifier
            JS_OBJ(groups, nodes);
        };

        QString type; // string
        std::optional<unsigned> radius; // ? uint
        std::optional<unsigned> radiusMin; // ? uint
        std::optional<QString> radiusVisual; // ? string
        std::optional<poe_api::ItemJewelData::Subgraph> subgraph; // ? object only present on cluster jewels
        JS_OBJ(type, radius, radiusMin, radiusVisual, subgraph);
    };

}
