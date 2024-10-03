#pragma once

#include <acquisition/api_types/character.h>
#include <acquisition/api_types/stash_tab.h>

#include <QObject>

#include <QSqlDatabase>
#include <QString>

class DataStore {

public:

    DataStore();

    void setCharacter(const poe_api::Character& character);
    void setStashTab(const poe_api::StashTab& stash_tab);

    poe_api::Character getCharacter(const QString& id);
    poe_api::StashTab getStashTab(const QString& id);

public slots:

    void setLeague(const QString& league);

private:

    void createTable(const QString& name, const QStringList& columns);

    QString m_data_directory;
    QString m_league;
    QSqlDatabase m_db;

};