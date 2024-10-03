#pragma once

#include <acquisition/api_types/pvp_ladder_team_member.h>

#include <json_struct/json_struct.h>

#include <optional>
#include <string>
#include <vector>

namespace poe_api{

    // https://www.pathofexile.com/developer/docs/reference#type-PvPLadderTeamEntry

    struct PvPLadderTeamEntry {

        unsigned rank; // uint
        std::optional<unsigned> rating; // ? uint only present if the PvP Match uses Glicko ratings
        std::optional<unsigned> points; // ? uint
        std::optional<unsigned> games_played; // ? uint
        std::optional<unsigned> cumulative_opponent_points; // ? uint
        std::optional<std::string> last_game_time; // ? string date time(ISO8601)
        std::vector<PvPLadderTeamMember> members; // array of PvPLadderTeamMember

        JS_OBJ(rank, rating, points, games_played, cumulative_opponent_points, last_game_time, members);

    };

}
