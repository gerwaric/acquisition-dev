#pragma once

#include <acquisition/utils/json.h>

#include <json_struct/json_struct.h>
#include <QsLog/QsLog.h>

#include <QObject>

#include <QDateTime>
#include <QSqlDatabase>
#include <QString>

class UserDatabase : public QObject {
    Q_OBJECT
public:
    UserDatabase(QObject* parent);

    void setInt(const QString& name, int value);
    int getInt(const QString& name);

    void setBool(const QString& name, bool value);
    bool getBool(const QString& name);

    void setString(const QString& name, const QString& value);
    QString getString(const QString& name);

    void setDateTime(const QString& name, const QDateTime& value);
    QDateTime getDateTime(const QString& name);

    template<typename T>
    void setStruct(const QString& name, const T& value)
    {
        const QString json_string = QString::fromStdString(JS::serializeStruct(value));
        setValue(name, json_string);
    }

    template<typename T>
    T getStruct(const QString& name)
    {
        T value;
        const std::string json_string = getValue(name).toString().toStdString();
        JS::ParseContext parseContext(json_string);
        if (parseContext.parseTo(value) != JS::Error::NoError) {
            QLOG_ERROR() << "Error parsing" << name << "from user database: " << parseContext.makeErrorString();
        };
        return value;
    };

public slots:

    void setUsername(const QString& username);

private:

    QVariant getValue(const QString& name);
    void setValue(const QString& name, const QVariant& value);

    QString m_username;
    QString m_league;
    QSqlDatabase m_db;

};