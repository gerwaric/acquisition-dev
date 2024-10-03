#include <acquisition/settings.h>

#include <array>
#include <map>

namespace {
    constexpr std::array<std::pair<QsLogging::Level, const char*>, 7> LOG_LEVELS = { {
        { QsLogging::Level::TraceLevel, "trace"},
        { QsLogging::Level::DebugLevel, "debug"},
        { QsLogging::Level::InfoLevel, "info"},
        { QsLogging::Level::WarnLevel, "warn"},
        { QsLogging::Level::ErrorLevel, "error"},
        { QsLogging::Level::FatalLevel, "fatal"},
        { QsLogging::Level::OffLevel, "off"}
    } };
}

Settings::Settings()
    : m_settings("new-settings.ini", QSettings::IniFormat) {}

QString Settings::sessionId()
{
    return m_settings.value("session_id").toString();
}

void Settings::setSessionId(const QString& sessionId)
{
    m_settings.setValue("session_id", sessionId);
}

QsLogging::Level Settings::logLevel()
{
    const QString value = m_settings.value("log_level", "debug").toString();
    for (const auto& pair : LOG_LEVELS) {
        const auto& level = pair.first;
        const auto& name = pair.second;
        if (value.compare(name, Qt::CaseInsensitive) == 0) {
            return level;
        };
    };
    return QsLogging::Level::DebugLevel;
}

void Settings::setLogLevel(QsLogging::Level value)
{
    for (const auto& pair : LOG_LEVELS) {
        const auto& level = pair.first;
        const auto& name = pair.second;
        if (value == level) {
            m_settings.setValue("log_level", name);
            return;
        };
    };
    QLOG_ERROR() << "Invalid value for log level:" << value;
}
