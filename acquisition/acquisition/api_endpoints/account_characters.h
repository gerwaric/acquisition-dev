#pragma once

#include <acquisition/api_types/character.h>

#include <QObject.h>
#include <QString>

#include <optional>
#include <vector>

namespace poe_api{

    // https://www.pathofexile.com/developer/docs/reference#characters-list
    std::vector<poe_api::Character> ListCharacters();

    // https://www.pathofexile.com/developer/docs/reference#stashes-get
    std::optional<poe_api::Character> GetCharacter(const QString& name);

}
