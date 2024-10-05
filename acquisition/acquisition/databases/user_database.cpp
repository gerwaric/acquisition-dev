#include <acquisition/databases/user_database.h>

#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>

UserDatabase::UserDatabase(QObject* parent) : QObject(parent)
{
}

void UserDatabase::setUsername(const QString& username)
{
    if (m_username == username) {
        return;
    };

    if (m_db.isOpen()) {
        m_db.close();
    };

    m_username = username;

    const QString& root = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    const QStringList parts = { root, "data", m_username, "user_data.sqlite3" };
    const QString filename = parts.join(QDir::separator());

    m_db = QSqlDatabase::addDatabase("QSQLITE", filename);
    m_db.setDatabaseName(filename);
    if (m_db.open() == false) {
        QLOG_ERROR() << "Failed to open QSQLITE database:" << filename << ":" << m_db.lastError().text();
        return;
    };

    QSqlQuery query(m_db);
    query.prepare("CREATE TABLE IF NOT EXISTS data (key TEXT PRIMARY KEY, value TEXT)");
    if (query.exec() == false) {
        QLOG_ERROR() << "League store failed to create character table:" << query.lastError().text();
    };
};

void UserDatabase::setInt(const QString& name, int value)
{
    setValue(name, value);
}

int UserDatabase::getInt(const QString& name)
{
    return getValue(name).toInt();
}

void UserDatabase::setBool(const QString& name, bool value)
{
    setValue(name, value);
}

bool UserDatabase::getBool(const QString& name)
{
    return getValue(name).toBool();
}

void UserDatabase::setString(const QString& name, const QString& value)
{
    setValue(name, value);
}

QString UserDatabase::getString(const QString& name)
{
    return getValue(name).toString();
}

void UserDatabase::setDateTime(const QString& name, const QDateTime& value)
{
    setValue(name, value.toString());
}

QDateTime UserDatabase::getDateTime(const QString& name)
{
    return QDateTime::fromString(getValue(name).toString());
}

void UserDatabase::setValue(const QString& name, const QVariant& value)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT OR REPLACE INTO data (key, value) VALUES (?, ?)");
    query.bindValue(0, name);
    query.bindValue(1, value);
    if (!query.exec()) {
        QLOG_ERROR() << "Error writing" << name << "to the user database:" << query.lastError().text();
    };
}

QVariant UserDatabase::getValue(const QString& name)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT value FROM data WHERE key = ?");
    query.bindValue(0, name);
    if (!query.exec()) {
        QLOG_ERROR() << "Error selecting" << name << "from user database:" << query.lastError().text();
        return QVariant();
    };
    if (!query.first()) {
        QLOG_ERROR() << "Error reading" << name << "from user database:" << query.lastError().text();
        return QVariant();
    };
    return query.value(0);
}
