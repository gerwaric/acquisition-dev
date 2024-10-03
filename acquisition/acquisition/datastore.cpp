#include <acquisition/datastore.h>

#include <QsLog/QsLog.h>

#include <QDateTime>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>

DataStore::DataStore()
{

}

void DataStore::setLeague(const QString& league)
{
    m_league = league;

    QString filedir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QString filename = "acquisition_data.db";
    QString filepath = filedir + QDir::separator() + filename;

    m_db = QSqlDatabase::addDatabase("QSQLITE", filepath);
    m_db.setDatabaseName(filename);
    if (m_db.open() == false) {
        QLOG_ERROR() << "Failed to open QSQLITE database:" << filename << ":" << m_db.lastError().text();
        return;
    };

    createTable("characters", {
            "character_id TEXT PRIMARY KEY",
            "last_update TEXT",
            "character_json BLOB"
        });

    createTable("stash_tabs", {
            "stash_id TEXT PRIMARY KEY",
            "last_update TEXT",
            "stash_json BLOB"
        });

    createTable("buyouts", {
            "item_id TEXT PRIMARY KEY",
            "last_update TEXT",
            "buyout_json BLOB)"
        });
}

void DataStore::createTable(const QString& name, const QStringList& columns)
{
    QString command = QString("CREATE TABLE IF NOT EXISTS %1 (%2)").arg(name, columns.join(", "));
    QSqlQuery query(m_db);
    query.prepare(command);
    if (query.exec() == false) {
        QLOG_ERROR() << "Failed to create" << name << "table :" << query.lastError().text();
    };
}


void DataStore::setCharacter(const poe_api::Character& character)
{
    const QString timestamp = QDateTime::currentDateTime().toLocalTime().toString();
    const QString json = QString::fromStdString(JS::serializeStruct(character));

    QSqlQuery query(m_db);
    query.prepare("INSERT OR REPLACE INTO characters (character_id, last_update, character_json) VALUES (?, ?)");
    query.bindValue(0, character.id);
    query.bindValue(1, timestamp);
    query.bindValue(2, json);
    if (query.exec() == false) {
        QLOG_ERROR() << QString("Error setting value for '%1' (%2) in '%3': %4").arg(
            character.name,
            character.id,
            m_league,
            query.lastError().text());
    };
}

void DataStore::setStashTab(const poe_api::StashTab& stash_tab)
{
    const QString timestamp = QDateTime::currentDateTime().toLocalTime().toString();
    const QString json = QString::fromStdString(JS::serializeStruct(stash_tab));

    QSqlQuery query(m_db);
    query.prepare("INSERT OR REPLACE INTO stash_tabs (stash_id, last_update, stash_json) VALUES (?, ?)");
    query.bindValue(0, stash_tab.id);
    query.bindValue(1, timestamp);
    query.bindValue(2, json);
    if (query.exec() == false) {
        QLOG_ERROR() << QString("Error setting value for '%1' (%2) in '%3': %4").arg(
            stash_tab.name,
            stash_tab.id,
            m_league,
            query.lastError().text());
    };
}

poe_api::Character DataStore::getCharacter(const QString& id)
{
    poe_api::Character character;

    QSqlQuery query(m_db);
    query.prepare("SELECT character_json FROM characters WHERE character_id = ?");
    query.bindValue(0, id);
    if (query.exec() == false) {
        QLOG_ERROR() << "Error selecting character with id" << id << ":" << query.lastError().text();
        return character;
    };
    if (query.next() == false) {
        if (query.isActive() == false) {
            QLOG_ERROR() << "Error getting character with id" << id << ":" << query.lastError().text();
        };
        return character;
    };

    const std::string bytes = query.value(0).toString().toStdString();
    JS::ParseContext parseContext(bytes);
    if (parseContext.parseTo(character) != JS::Error::NoError) {
        QLOG_ERROR() << "Error parsing character:" << parseContext.makeErrorString();
    };
    return character;
}

poe_api::StashTab DataStore::getStashTab(const QString& id)
{
    poe_api::StashTab stash_tab;

    QSqlQuery query(m_db);
    query.prepare("SELECT stah_json FROM stash_tabs WHERE stash_id = ?");
    query.bindValue(0, id);
    if (query.exec() == false) {
        QLOG_ERROR() << "Error selecting stash tab with id" << id << ":" << query.lastError().text();
        return stash_tab;
    };
    if (query.next() == false) {
        if (query.isActive() == false) {
            QLOG_ERROR() << "Error getting stash tab with id" << id << ":" << query.lastError().text();
        };
        return stash_tab;
    };

    const std::string bytes = query.value(0).toString().toStdString();
    JS::ParseContext parseContext(bytes);
    if (parseContext.parseTo(stash_tab) != JS::Error::NoError) {
        QLOG_ERROR() << "Error parsing stash tab:" << parseContext.makeErrorString();
    };
    return stash_tab;
}