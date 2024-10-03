#include <acquisition/api_endpoints/account_characters.h>

std::vector<poe_api::Character> poe_api::ListCharacters()
{
    return {};
}

std::optional<poe_api::Character> poe_api::GetCharacter(const QString& name)
{
    Q_UNUSED(name);
    return {};
}
