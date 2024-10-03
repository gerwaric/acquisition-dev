#pragma once

#include <json_struct/json_struct.h>

#include <acquisition/utils/json.h>
#include <acquisition/api_types/league_rule.h>

#include <QString>

#include <optional>
#include <vector>

namespace poe_api{

    // https://www.pathofexile.com/developer/docs/reference#type-League

    struct League {

        struct Category {
            QString                          id;             // string the league category, e.g.Affliction
            std::optional<bool>                  current;        // ? bool set for the active challenge leagues; always true if present
            JS_OBJ(id, current);
        };

        QString id;                                         // string the league's name
        std::optional<QString>                    realm;          // ? string pc, xbox, or sony
        std::optional<QString>                    description;    // ? string
        std::optional<poe_api::League::Category>     category;       // ? object
        std::optional<std::vector<poe_api::LeagueRule>>   rules;          // ? array of LeagueRule
        std::optional<QString>                    registerAt;     // ? string date time(ISO8601)
        std::optional<bool>                      event;          // ? bool always true if present
        std::optional<QString>                    url;            // ? string a url link to a Path of Exile forum thread
        std::optional<QString>                    startAt;        // ? string date time(ISO8601)
        std::optional<QString>                    endAt;          // ? string date time(ISO8601)
        std::optional<bool>                      timedEvent;     // ? bool always true if present
        std::optional<bool>                      scoreEvent;     // ? bool always true if present
        std::optional<bool>                      delveEvent;     // ? bool always true if present
        std::optional<bool>                      ancestorEvent;  // ? bool always true if present
        std::optional<bool>                      leagueEvent;    // ? bool always true if present

        JS_OBJ(id, realm, description, category, rules, registerAt, event, url, startAt,
            endAt, timedEvent, scoreEvent, delveEvent, ancestorEvent, leagueEvent);

    };

}
