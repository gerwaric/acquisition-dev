#include <acquisition/settings.h>

#include <acquisition/utils/utils.h>

#include <QDir>
#include <QStandardPaths>

#include <array>
#include <map>

Settings::Settings(QObject* parent, const QString& data_directory) : QObject(parent)
{
    const QString filename = data_directory + QDir::separator() + "settings-new.ini";
    m_settings = std::make_unique<QSettings>(filename, QSettings::IniFormat);
}

void Settings::sendSignals() const
{
    if (!username().isEmpty()) {
        emit usernameChanged(username());
    };
    if (!league().isEmpty()) {
        emit leagueChanged(league());
    };
    if (!sessionId().isEmpty()) {
        emit sessionIdChanged(sessionId());
    };
}

QString Settings::username() const
{
    return m_settings->value("username").toString();
}

QString Settings::league() const
{
    return m_settings->value("username").toString();
}

QString Settings::sessionId() const
{
    return m_settings->value("session_id").toString();
}

QsLogging::Level Settings::logLevel() const
{
    const QString name = m_settings->value("log_level", "DEBUG").toString();
    return utils::logLevel(name);
}

void Settings::setUsername(const QString& username)
{
    const QString previous_value = m_settings->value("username").toString();
    m_settings->setValue("username", username);
    if (previous_value != username) {
        emit usernameChanged(username);
    };
}

void Settings::setLeague(const QString& league)
{
    const QString previous_value = m_settings->value("league").toString();
    m_settings->setValue("league", league);
    if (previous_value != league) {
        emit leagueChanged(league);
    };
}

void Settings::setSessionId(const QString& session_id)
{
    const QString previous_value = m_settings->value("session_id").toString();
    m_settings->setValue("session_id", session_id);
    if (previous_value != session_id) {
        emit sessionIdChanged(session_id);
    };
}

void Settings::setLogLevel(QsLogging::Level level)
{
    const QString previous_value = m_settings->value("logging_level").toString();
    const QString next_value = utils::logLevelName(level);
    if (0 != previous_value.compare(next_value, Qt::CaseInsensitive)) {
        m_settings->setValue("log_level", next_value);
        emit logLevelChanged(level);
    };
}
