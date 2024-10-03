#pragma once

#include <acquisition/api_types/stash_tab.h>

#include <optional>
#include <string>
#include <vector>

namespace poe_api {

    // https://www.pathofexile.com/developer/docs/reference#stashes-list
    std::vector<poe_api::StashTab> ListStashes(
        const std::string& league);

    // https://www.pathofexile.com/developer/docs/reference#stashes-get
    std::optional<poe_api::StashTab> GetStash(
        const std::string& league,
        const std::string& stash_id,
        const std::string& substash_id = "");

}
