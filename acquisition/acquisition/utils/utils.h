#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QMetaEnum>

#include <string>
#include <string_view>

namespace utils {

    inline bool ends_with(std::string_view str, std::string_view suffix)
    {
        return (str.size() >= suffix.size())
            && (str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0);
    };

    inline bool starts_with(std::string_view str, std::string_view prefix)
    {
        return (str.size() >= prefix.size())
            && (str.compare(0, prefix.size(), prefix) == 0);
    };

    QByteArray replace_obsolete_timezones(const QByteArray& rfc2822_date);

    std::string replace_obsolete_timezones(const std::string& date);

    QDateTime parseRFC2822(const QByteArray& rfc2822_date);

    QDateTime parseRFC2822(const std::string& rfc2822_date);

    template<typename QEnum>
    QString QtEnumToString(const QEnum value)
    {
        return QMetaEnum::fromType<QEnum>().valueToKey(static_cast<int>(value));    
    }

    template<typename QEnum>
    QString QtEnumToStdString(const QEnum value)
    {
        return QtEnumToString(value).toStdString();
    }
}
