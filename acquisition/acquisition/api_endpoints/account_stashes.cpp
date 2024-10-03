#include <acquisition/api_endpoints/account_stashes.h>

#include <acquisition/api_types/stash_tab.h>

std::vector<poe_api::StashTab> poe_api::ListStashes(
    const std::string& league)
{
    Q_UNUSED(league);
    return {};
}

std::optional<poe_api::StashTab> poe_api::GetStash(
    const std::string& league,
    const std::string& stash_id,
    const std::string& substash_id)
{
    Q_UNUSED(league);
    Q_UNUSED(stash_id);
    Q_UNUSED(substash_id);
    return {};
}
