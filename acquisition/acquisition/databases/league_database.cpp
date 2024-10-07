#include <acquisition/databases/league_database.h>

#include <QsLog/QsLog.h>

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>

LeagueDatabase::LeagueDatabase(QObject* parent, const QString& data_directory)
    : QObject(parent)
    , m_data_directory(data_directory) {}

void LeagueDatabase::setUsername(const QString& username)
{
    if (m_username == username) {
        return;
    };
    m_username = username;
    if (m_db.isOpen()) {
        m_db.close();
        setLeague(m_league);
    };
}

void LeagueDatabase::setLeague(const QString& league)
{
    if (m_league == league) {
        if (m_db.isOpen()) {
            return;
        };
    };

    if (m_db.isOpen()) {
        m_db.close();
    };

    m_league = league;

    const QStringList parts = { m_data_directory, m_username, "league_data", (m_league + ".sqlite3")};
    const QString filename = parts.join(QDir::separator());
    m_db = QSqlDatabase::addDatabase("QSQLITE", filename);
    m_db.setDatabaseName(filename);
    if (m_db.open() == false) {
        QLOG_ERROR() << "Failed to open QSQLITE database:" << filename << ":" << m_db.lastError().text();
        return;
    };

    QSqlQuery query(m_db);
    
    query.prepare("CREATE TABLE IF NOT EXISTS characters ("
        "id TEXT PRIMARY KEY, "
        "name TEXT, "
        "data TEXT, "
        "hash TEXT, "
        "timestamp TEXT)");
    
    if (query.exec() == false) {
        QLOG_ERROR() << "League store failed to create character table:" << query.lastError().text();
    };
    
    query.prepare("CREATE TABLE IF NOT EXISTS stash_tabs ("
        "id TEXT PRIMARY KEY, "
        "name TEXT, "
        "type TEXT, "
        "parent TEXT, "
        "data TEXT, "
        "hash TEXT, "
        "timestamp TEXT)");
    
    if (query.exec() == false) {
        QLOG_ERROR() << "League store failed to create stash_tabs table:" << query.lastError().text();
    };

    query.prepare("CREATE TABLE IF NOT EXISTS buyouts ("
        "id TEXT PRIMARY KEY, "
        "last_update TEXT, "
        "data TEXT)");
    
    if (query.exec() == false) {
        QLOG_ERROR() << "League store failed to create buyout table:" << query.lastError().text();
    };
}

QStringList LeagueDatabase::getCharacterList()
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

QStringList LeagueDatabase::getStashList()
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

std::shared_ptr<poe_api::Character> LeagueDatabase::getCharacter(const QString& id)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT data FROM characters WHERE id = ?");
    query.bindValue(0, id);
    if (!query.exec()) {
        QLOG_ERROR() << "Error selecting character with id" << id << ":" << query.lastError().text();
        return nullptr;
    };
    if (!query.first()) {
        QLOG_ERROR() << "Error getting character with id" << id << ":" << query.lastError().text();
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

std::shared_ptr<poe_api::StashTab> LeagueDatabase::getStash(const QString& id)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT data FROM stash_tabs WHERE id = ?");
    query.bindValue(0, id);
    if (query.exec()) {
        QLOG_ERROR() << "Error selecting stash tab with id" << id << ":" << query.lastError().text();
        return nullptr;
    };
    if (!query.first()) {
        QLOG_ERROR() << "Error getting stash tab with id" << id << ":" << query.lastError().text();
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

void LeagueDatabase::storeCharacter(const poe_api::Character& character)
{       
    const QString json = QString::fromStdString(JS::serializeStruct(character));
    const QString hash = generateHash(json);
    const QString timestamp = QDateTime::currentDateTime().toLocalTime().toString();

    QSqlQuery query(m_db);
    query.prepare("INSERT OR REPLACE INTO characters (id, name, data, hash, timestamp) "
        "VALUES (:id, :name, :data, :hash, :timestamp)");
    query.bindValue(":id", character.id);
    query.bindValue(":name", character.name);
    query.bindValue(":data", json);
    query.bindValue(":hash", hash);
    query.bindValue(":timestamp", timestamp);

    if (query.exec() == false) {
        QLOG_ERROR() << "Error storing character" << character.name << ":" << query.lastError().text();
    };
}

void LeagueDatabase::storeStash(const poe_api::StashTab& stash_tab)
{
    const QString json = QString::fromStdString(JS::serializeStruct(stash_tab));
    const QString hash = generateHash(json);
    const QString timestamp = QDateTime::currentDateTime().toLocalTime().toString();

    QSqlQuery query(m_db);
    query.prepare("INSERT OR REPLACE INTO stash_tabs (id, name, type, parent, data, hash, timestamp) "
        "VALUES (:id, :name, :type, :parent, :data, :hash, :timestamp)");
    query.bindValue(":id", stash_tab.id);
    query.bindValue(":name", stash_tab.name);
    query.bindValue(":type", stash_tab.type);
    query.bindValue(":parent", stash_tab.parent.value_or(""));
    query.bindValue(":data", json);
    query.bindValue(":hash", hash);
    query.bindValue(":timestamp", timestamp);

    if (query.exec() == false) {
        QLOG_ERROR() << "Error storing stash tab" << stash_tab.name << ":" << query.lastError().text();
    };
}

QString LeagueDatabase::generateHash(const QString& data)
{
    const auto algorithm = QCryptographicHash::Blake2b_256;
    const QByteArray bytes = data.toUtf8();
    const QByteArray hash = QCryptographicHash::hash(data.toUtf8(), algorithm);
    return QString(hash.toHex());
}