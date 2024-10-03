#pragma once

#include <json_struct/json_struct.h>

#include <acquisition/utils/json.h>
#include <acquisition/api_types/item.h>
#include <acquisition/api_types/item_jewel_data.h>
#include <acquisition/api_types/passive_node.h>

#include <QString>

#include <optional>
#include <unordered_map>
#include <vector>

namespace poe_api {

    // https://www.pathofexile.com/developer/docs/reference#type-Character

    struct Character {

        struct Passives {
            std::vector<unsigned> hashes; // array of uint
            std::vector<unsigned> hashes_ex; // array of uint
            std::unordered_map<QString, int> mastery_effects; // dictionary of int the key is the string value of the mastery node skill hash and the value is the selected effect hash
            std::unordered_map<QString, poe_api::PassiveNode> skill_overrides; // dictionary of PassiveNode the key is the string value of the node identifier being replaced
            std::optional<QString> bandit_choice; // ? string one of Kraityn, Alira, Oak, or Eramir
            std::optional<QString> pantheon_major; // ? string one of TheBrineKing, Arakaali, Solaris, or Lunaris
            std::optional<QString> pantheon_minor; // ? string one of Abberath, Gruthkul, Yugul, Shakari, Tukohama, Ralakesh, Garukhan, or Ryslatha
            std::unordered_map<QString, poe_api::ItemJewelData> jewel_data; // dictionary of ItemJewelData the key is the string value of the x property of an item from the jewels array in this request
            std::optional<QString> alternate_ascendancy; // ? string Warden, Warlock, or Primalist
            JS_OBJ(hashes, hashes_ex, mastery_effects, skill_overrides, bandit_choice, pantheon_major, pantheon_minor, //jewel_data,
                alternate_ascendancy);
        };

        struct Metadata {
            std::optional<QString> version; // ? string game version for the character's realm
            JS_OBJ(version);
        };

        QString id; // string a unique 64 digit hexadecimal string
        QString name; // string
        QString realm; // string pc, xbox, or sony
        QString class_; // string
        std::optional<QString> league; // ? string
        unsigned level; // uint
        unsigned experience; // uint
        std::optional<bool> ruthless; // ? bool always true if present
        std::optional<bool> expired; // ? bool always true if present
        std::optional<bool> deleted; // ? bool always true if present
        std::optional<bool> current; // ? bool always true if present
        std::optional<std::vector<poe_api::Item>> equipment; // ? array of Item
        std::optional<std::vector<poe_api::Item>> inventory; // ? array of Item
        std::optional<std::vector<poe_api::Item>> rucksack; // ? array of Item items stored in the Primalist's Rucksack
        std::optional<std::vector<poe_api::Item>> jewels; // ? array of Item
        std::optional<poe_api::Character::Passives> passives; // ? object
        std::optional<poe_api::Character::Metadata> metadata; // ? object

        JS_OBJECT(
            JS_MEMBER(id),
            JS_MEMBER(name),
            JS_MEMBER(realm),
            JS_MEMBER_WITH_NAME(class_, "class"),
            JS_MEMBER(league),
            JS_MEMBER(level),
            JS_MEMBER(experience),
            JS_MEMBER(ruthless),
            JS_MEMBER(expired),
            JS_MEMBER(deleted),
            JS_MEMBER(current),
            JS_MEMBER(equipment),
            JS_MEMBER(inventory),
            JS_MEMBER(rucksack),
            JS_MEMBER(jewels),
            JS_MEMBER(passives),
            JS_MEMBER(metadata));
    };

}
