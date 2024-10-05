#pragma once

#include <QsLog/QsLog.h>

#include <QObject>

#include <QSettings>

class Settings : public QObject {
    Q_OBJECT
public:
    Settings(QObject* parent, const QString& data_directory);
    QString username() const;
    QString league() const;
    QString sessionId() const;
    QsLogging::Level logLevel() const;

    void sendSignals() const;

signals:
    void usernameChanged(const QString& username) const;
    void leagueChanged(const QString& league) const;
    void sessionIdChanged(const QString& session_id) const;
    void logLevelChanged(QsLogging::Level level) const;

public slots:
    void setUsername(const QString& username);
    void setLeague(const QString& league);
    void setSessionId(const QString& session_id);
    void setLogLevel(QsLogging::Level level);

private:
    std::unique_ptr<QSettings> m_settings;
};