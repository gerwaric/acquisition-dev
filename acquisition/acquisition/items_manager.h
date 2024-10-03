#pragma once

#include <acquisition/api_types/character.h>
#include <acquisition/api_types/stash_tab.h>

#include <QObject>

#include <vector>

class ItemsManager : public QObject {
    Q_OBJECT
public:

    ItemsManager();

public slots:

    void receiveCharacterList(std::vector<poe_api::Character> characters);
    void receiveCharacter(poe_api::Character character);
    void receiveStashList(std::vector<poe_api::StashTab> stash_tabs);
    void receiveStash(poe_api::StashTab stash_tab);

private:

    std::vector<poe_api::Character> character_list;
    std::vector<poe_api::StashTab> stash_list;

};