#include <acquisition/items_manager.h>

ItemsManager::ItemsManager()
{

}

void ItemsManager::receiveCharacterList(std::vector<poe_api::Character> characters)
{
    character_list = characters;
}

void ItemsManager::receiveCharacter(poe_api::Character character)
{

}

void ItemsManager::receiveStashList(std::vector<poe_api::StashTab> stashes)
{
    stash_list = stashes;
}

void ItemsManager::receiveStash(poe_api::StashTab stash)
{

}
