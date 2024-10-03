#pragma once

#include <acquisition/api_types/character.h>
#include <acquisition/api_types/stash_tab.h>

#include <QObject>

#include <QSqlDatabase>
#include <QString>
#include <QStringList>

class LeagueDataStore {

public:

    LeagueDataStore();

    QStringList getCharacterList();
    QStringList getStashList();

    poe_api::Character getCharacter(const QString& id);
    poe_api::StashTab getStash(const QString& id);

    void storeCharacter(const poe_api::Character& character);
    void storeStash(const poe_api::StashTab& stash_tab);

public slots:

    void setLeague(const QString& league);

private:

    void createTable(const QString& name, const QStringList& columns);
    QStringList getList(const QString& column, const QString& table);
    QStringList getSingleValue(const QString& column, const QString& table, const QString& key);

    QString m_data_directory;
    QString m_league;
    QSqlDatabase m_db;

};