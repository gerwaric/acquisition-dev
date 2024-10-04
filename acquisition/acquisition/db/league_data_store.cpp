#include <acquisition/db/league_data_store.h>

#include <QsLog/QsLog.h>

#include <QDateTime>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>

LeagueDataStore::LeagueDataStore()
{

}

void LeagueDataStore::setLeague(const QString& league)
{
    if (m_db.isOpen()) {
        m_db.close();
    };

    m_league = league;

    const QStringList parts = {
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation),
        "league_data",
        m_league + ".db"
    };
    const QString filename = parts.join(QDir::separator());

    m_db = QSqlDatabase::addDatabase("QSQLITE", filename);
    m_db.setDatabaseName(filename);
    if (m_db.open() == false) {
        QLOG_ERROR() << "Failed to open QSQLITE database:" << filename << ":" << m_db.lastError().text();
        return;
    };

    QSqlQuery query(m_db);

    query.prepare("CREATE TABLE IF NOT EXISTS characters (id TEXT PRIMARY KEY, last_update TEXT, data JSON)");
    if (query.exec() == false) {
        QLOG_ERROR() << "League store failed to create character table:" << query.lastError().text();
    };

    query.prepare("CREATE TABLE IF NOT EXISTS stash_tabs (id TEXT PRIMARY KEY, last_update TEXT, data JSON)");
    if (query.exec() == false) {
        QLOG_ERROR() << "League store failed to create stash_tabs table:" << query.lastError().text();
    };

    query.prepare("CREATE TABLE IF NOT EXISTS buyouts (id TEXT PRIMARY KEY, last_update TEXT, data JSON)");
    if (query.exec() == false) {
        QLOG_ERROR() << "League store failed to create buyout table:" << query.lastError().text();
    };
}

QStringList LeagueDataStore::getCharacterList()
{
    QSqlQuery query(m_db);
    query.prepare("SELECT id FROM characters");
    if (!query.exec()) {
        QLOG_ERROR() << "Error selecting characters:" << query.lastError().text();
        return {};
    };

    QStringList character_ids;
    int i = 0;
    while (query.next()) {
        ++i;
        if (query.lastError().isValid()) {
            QLOG_ERROR() << "Error getting character_id" << i << ":" << query.lastError().text();
            break;
        };
        character_ids.push_back(query.value(0).toString());
    };
    return character_ids;
}

QStringList LeagueDataStore::getStashList()
{
    QSqlQuery query(m_db);
    query.prepare("SELECT id FROM stash_tabs");
    if (!query.exec()) {
        QLOG_ERROR() << "Error selecting characters:" << query.lastError().text();
        return {};
    };

    QStringList stash_ids;
    int i = 0;
    while (query.next()) {
        ++i;
        if (query.lastError().isValid()) {
            QLOG_ERROR() << "Error getting stash_id" << i << ":" << query.lastError().text();
            break;
        };
        stash_ids.push_back(query.value(0).toString());
    };
    return stash_ids;
}

std::shared_ptr<poe_api::Character> LeagueDataStore::getCharacter(const QString& id)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT data FROM characters WHERE id = ?");
    query.bindValue(0, id);
    if (query.exec()) {
        QLOG_ERROR() << "Error selecting character with id" << id << ":" << query.lastError().text();
        return nullptr;
    };

    if (!query.next()) {
        if (!query.isActive()) {
            QLOG_ERROR() << "Error getting character with id" << id << ":" << query.lastError().text();
        };
        return nullptr;
    };

    auto character = std::make_shared<poe_api::Character>();
    const std::string bytes = query.value(0).toString().toStdString();
    JS::ParseContext parseContext(bytes);
    if (parseContext.parseTo(character) != JS::Error::NoError) {
        QLOG_ERROR() << "Error parsing character:" << parseContext.makeErrorString();
    };
    return character;
}

std::shared_ptr<poe_api::StashTab> LeagueDataStore::getStash(const QString& id)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT data FROM stash_tabs WHERE id = ?");
    query.bindValue(0, id);
    if (query.exec()) {
        QLOG_ERROR() << "Error selecting stash tab with id" << id << ":" << query.lastError().text();
        return nullptr;
    };

    if (!query.next()) {
        if (!query.isActive()) {
            QLOG_ERROR() << "Error getting stash tab with id" << id << ":" << query.lastError().text();
        };
        return nullptr;
    };

    auto stash_tab = std::make_shared<poe_api::StashTab>();
    const std::string bytes = query.value(0).toString().toStdString();
    JS::ParseContext parseContext(bytes);
    if (parseContext.parseTo(stash_tab) != JS::Error::NoError) {
        QLOG_ERROR() << "Error parsing character:" << parseContext.makeErrorString();
    };
    return stash_tab;
}


void LeagueDataStore::storeCharacter(const poe_api::Character& character)
{
    const QString timestamp = QDateTime::currentDateTime().toLocalTime().toString();
    const QString json = QString::fromStdString(JS::serializeStruct(character));

    QSqlQuery query(m_db);
    query.prepare("INSERT OR REPLACE INTO characters (id, last_update, data) VALUES (?, ?, ?)");
    query.bindValue(0, character.id);
    query.bindValue(1, timestamp);
    query.bindValue(2, json);

    if (query.exec() == false) {
        QLOG_ERROR() << "Error storing character" << character.name << ":" << query.lastError().text();
    };
}

void LeagueDataStore::storeStash(const poe_api::StashTab& stash_tab)
{
    const QString timestamp = QDateTime::currentDateTime().toLocalTime().toString();
    const QString json = QString::fromStdString(JS::serializeStruct(stash_tab));

    QSqlQuery query(m_db);
    query.prepare("INSERT OR REPLACE INTO stash_tabs (id, last_update, data) VALUES (?, ?, ?)");
    query.bindValue(0, stash_tab.id);
    query.bindValue(1, timestamp);
    query.bindValue(2, json);

    if (query.exec() == false) {
        QLOG_ERROR() << "Error storing character" << stash_tab.name << ":" << query.lastError().text();
    };
}
