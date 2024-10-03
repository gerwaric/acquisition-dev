#pragma once

#include <QsLog/QsLog.h>

#include <QSettings>

class Settings
{
public:

    Settings();

    QString sessionId();
    void setSessionId(const QString& sessionId);

    QsLogging::Level logLevel();
    void setLogLevel(QsLogging::Level level);

private:

    QSettings m_settings;

};