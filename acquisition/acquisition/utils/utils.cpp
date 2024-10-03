#include <acquisition/utils/utils.h>

namespace {
    
}

namespace utils {

    QByteArray replace_obsolete_timezones(const QByteArray& date)
    {
        // Qt 6.5.3 does not properly parse the non-numeric time-zones that are 
        // declated as obsolete but still allowed in RFC2822.
        constexpr std::array<std::pair<const char*, const char*>, 10> OBSOLETE_TIMEZONES = { {
            {"GMT", "+0000"},
            {"UT" , "+0000"},
            {"EST", "-0005"},
            {"EDT", "-0004"},
            {"CST", "-0006"},
            {"CDT", "-0005"},
            {"MST", "-0007"},
            {"MDT", "-0006"},
            {"PST", "-0008"},
            {"PDT", "-0007"}} };
        QByteArray fixed = date.trimmed();
        for (const auto& pair : OBSOLETE_TIMEZONES) {
            const char* zone = pair.first;
            const char* offset = pair.second;
            if (fixed.endsWith(zone)) {
                fixed = date.chopped(std::strlen(zone)) + offset;
            };
        };
        return fixed;
    }

    std::string replace_obsolete_timezones(const std::string& date)
    {
        return utils::replace_obsolete_timezones(QByteArray::fromStdString(date)).toStdString();
    }

    QDateTime parseRFC2822(const QByteArray& date) {
        const QByteArray fixed_date = replace_obsolete_timezones(date);
        return QDateTime::fromString(fixed_date, Qt::RFC2822Date);
    }

    QDateTime parseRFC2822(const std::string& date) {
        return parseRFC2822(QByteArray::fromStdString(date));
    }

}