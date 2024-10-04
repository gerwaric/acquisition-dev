/*
    Copyright 2023 Gerwaric

    This file is part of Acquisition.

    Acquisition is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Acquisition is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Acquisition.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <acquisition/rate_limit/rate_limit_policy.h>

#include <acquisition/utils/utils.h>

#include <QsLog/QsLog.h>

#include <QNetworkReply>
#include <QUrl>

//=========================================================================================
// RateLimitPolicy::Element
//=========================================================================================

RateLimitPolicy::Element::Element(const QByteArray& header_fragment)
{
    const QByteArrayList parts = header_fragment.split(':');
    hits = parts[0].toInt();
    period = parts[1].toInt();
    restriction = parts[2].toInt();
}

//=========================================================================================
// RateLimitPolicy::Item
//=========================================================================================

RateLimitPolicy::Item::Item(const QByteArray& limit_fragment, const QByteArray& state_fragment)
    : m_limit(limit_fragment)
    , m_state(state_fragment)
{
    m_maximum_hits = m_limit.hits;
    if (m_state.period != m_limit.period) {
        m_status = RateLimitPolicy::Status::Invalid;
    } else if (m_state.hits > m_limit.hits) {
        m_status = RateLimitPolicy::Status::Violation;
    } else if (m_state.hits == m_limit.hits) {
        m_status = RateLimitPolicy::Status::Borderline;
    } else {
        m_status = RateLimitPolicy::Status::Ok;
    };
}

void RateLimitPolicy::Item::Check(const RateLimitPolicy::Item& other, const QString& prefix) const
{
    if (m_limit.hits != other.m_limit.hits) {
        QLOG_WARN() << prefix << "limit.hits changed"
            << "from" << m_limit.hits
            << "to" << other.m_limit.hits;
    };
    if (m_limit.period != other.m_limit.period) {
        QLOG_WARN() << prefix << "limit.period changed"
            << "from" << m_limit.period
            << "to" << other.m_limit.period;
    };
    if (m_limit.restriction != other.m_limit.restriction) {
        QLOG_WARN() << prefix << "limit.restriction changed"
            << "from" << m_limit.restriction
            << "to" << other.m_limit.restriction;
    };
}

QDateTime RateLimitPolicy::Item::GetNextSafeSend(const boost::circular_buffer<QDateTime>& history) const
{
    QLOG_TRACE() << "RateLimit::RuleItem::GetNextSafeSend() entered";

    const QDateTime now = QDateTime::currentDateTime().toLocalTime();

    // We can send immediately if we have not bumped up against a rate limit.
    if (m_state.hits < m_limit.hits) {
        return now;
    };

    // Determine how far back into the history we can look.
    const size_t n = (m_limit.hits > history.size()) ? history.size() : m_limit.hits;

    // Start with the timestamp of the earliest known 
    // reply relevant to this limitation.
    const QDateTime earliest = (n < 1) ? now : history[n - 1];

    const QDateTime next_send = earliest.addSecs(m_limit.period);

    QLOG_TRACE() << "RateLimit::RuleItem::GetNextSafeSend()"
        << "n =" << n
        << "earliest =" << earliest.toString() << "(" << earliest.secsTo(now) << "seconds ago)"
        << "next_send =" << next_send.toString() << "(in" << now.secsTo(next_send) << "seconds)";

    // Calculate the next time it will be safe to send a request.
    return next_send;
}

int RateLimitPolicy::Item::EstimateDuration(int request_count, int minimum_delay_msec) const
{
    QLOG_TRACE() << "RateLimit::RuleItem::EstimateDuration() entered";

    int duration = 0;

    const int current_hits = m_state.hits;
    const int max_hits = m_limit.hits;
    const int period_length = m_limit.period;
    const int restriction = m_limit.restriction;

    int initial_burst = max_hits - current_hits;
    if (initial_burst < 0) {
        initial_burst = 0;
        duration += restriction;
    };

    int remaining_requests = request_count - initial_burst;
    if (remaining_requests < 0) {
        remaining_requests = 0;
    };

    const int full_periods = (remaining_requests / max_hits);
    const int final_burst = (remaining_requests % max_hits);

    const int a = initial_burst * minimum_delay_msec;
    const int b = full_periods * period_length * 1000;
    const int c = final_burst * minimum_delay_msec;
    const int total_msec = a + b + c;

    duration += (total_msec / 1000);

    return duration;
}

//=========================================================================================
// RateLimitPolicy::Rule
//=========================================================================================

RateLimitPolicy::Rule::Rule(const QByteArray& rule_name, QNetworkReply* const reply)
    : m_name(rule_name)
{
    QLOG_TRACE() << "RateLimit::PolicyRule::PolicyRule() entered";
    const QByteArrayList m_limitfragments = parseRateLimit(reply, rule_name);
    const QByteArrayList m_statefragments = parseRateLimitState(reply, rule_name);
    const int item_count = m_limitfragments.size();
    if (m_statefragments.size() != m_limitfragments.size()) {
        QLOG_ERROR() << "Invalid data for policy role.";
    };
    m_items.reserve(item_count);
    for (int j = 0; j < item_count; ++j) {
        const auto& item = m_items.emplace_back(m_limitfragments[j], m_statefragments[j]);
        if (m_maximum_hits < item.maximum_hits()) {
            m_maximum_hits = item.maximum_hits();
        };
        if (m_status < item.status()) {
            m_status = item.status();
        };
    };
}

void RateLimitPolicy::Rule::Check(const RateLimitPolicy::Rule& other, const QString& prefix) const
{
    QLOG_TRACE() << "RateLimit::PolicyRule::Check() entered";

    // Check the rule name
    if (m_name != other.name()) {
        QLOG_WARN() << prefix << "rule name changed"
            << "from" << m_name << "to" << other.name();
    };

    // Check the number of items in this rule
    if (m_items.size() != other.items().size()) {

        // The number of items changed
        QLOG_WARN() << prefix << "rule" << m_name << "went"
            << "from" << m_items.size() << "items"
            << "to" << other.items().size() << "items";

    } else {

        // Check each item
        for (int i = 0; i < m_items.size(); ++i) {
            const QString item_prefix = QString("%1 item #%2").arg(prefix, QString::number(i));
            const auto& old_item = m_items[i];
            const auto& new_item = other.items()[i];
            old_item.Check(new_item, item_prefix);
        };

    };
}

//=========================================================================================
// RateLimitPolicy
//=========================================================================================

RateLimitPolicy::RateLimitPolicy(QNetworkReply* const reply)
{
    m_name = QString(parseRateLimitPolicy(reply));

    const QByteArrayList names = parseRateLimitRules(reply);
    m_rules.reserve(names.size());
    for (const auto& name : names) {
        const auto& rule = m_rules.emplace_back(name, reply);
        if (m_maximum_hits < rule.maximum_hits()) {
            m_maximum_hits = rule.maximum_hits();
        };
        if (m_status < rule.status()) {
            m_status = rule.status();
        };
    };
}

void RateLimitPolicy::RateLimitPolicy::Check(const RateLimitPolicy& other) const
{
    QLOG_TRACE() << "RateLimit::Policy::Check() entered";

    // Check the policy name
    if (m_name != other.name()) {
        QLOG_WARN() << "The rate limit policy name change from" << m_name << "to" << other.name();
    };

    // Check the number of rules
    if (m_rules.size() != other.rules().size()) {

        // The number of rules changed
        QLOG_WARN() << "The rate limit policy" << m_name
            << "had" << m_rules.size() << "rules,"
            << "but now has" << other.rules().size();

    } else {

        // The number of rules is the same, so check each one
        for (int i = 0; i < m_rules.size(); ++i) {
            const QString prefix = QString("Rate limit policy %1, rule #%2:").arg(m_name, QString::number(i));
            const auto& old_rule = m_rules[i];
            const auto& new_rule = other.rules()[i];
            old_rule.Check(new_rule, prefix);
        };

    };
}

QDateTime RateLimitPolicy::RateLimitPolicy::GetNextSafeSend(const boost::circular_buffer<QDateTime>& history)
{
    QLOG_TRACE() << "RateLimit::Policy::GetNextSafeSend() entered";

    QDateTime next_send = QDateTime::currentDateTime().toLocalTime();
    for (const auto& rule : m_rules) {
        for (const auto& item : rule.items()) {
            const QDateTime t = item.GetNextSafeSend(history);
            QLOG_TRACE() << "RateLimit::Policy::GetNextSafeSend()"
                << m_name << rule.name() << "t =" << t.toString()
                << "(in" << QDateTime::currentDateTime().secsTo(t) << "seconds)";
            if (next_send < t) {
                QLOG_TRACE() << "RateLimit::Policy::GetNextSafeSend() updating next_send";
                next_send = t;
            };
        };
    };
    return next_send;
}

QDateTime RateLimitPolicy::RateLimitPolicy::EstimateDuration(int num_requests, int minimum_delay_msec) const
{
    QLOG_TRACE() << "RateLimit::Policy::EstimateDuration() entered";

    int longest_wait = 0;
    while (true) {
        for (const auto& rule : m_rules) {
            for (const auto& item : rule.items()) {
                const int wait = item.EstimateDuration(num_requests, minimum_delay_msec);
                if (longest_wait < wait) {
                    longest_wait = wait;
                };
            };
        };
    };
    return QDateTime::currentDateTime().toLocalTime().addSecs(longest_wait);
}

// Get a header field from an HTTP reply.
QByteArray RateLimitPolicy::parseHeader(QNetworkReply* const reply, const QByteArray& name)
{
    if (!reply->hasRawHeader(name)) {
        QLOG_ERROR() << "RateLimit: the network reply is missing a header:" << name;
    };
    return reply->rawHeader(name);
}

// Get a header field and split into a list.
QByteArrayList RateLimitPolicy::parseHeaderList(QNetworkReply* const reply, const QByteArray& name, const char delim)
{
    const QByteArray value = parseHeader(reply, name);
    const QByteArrayList items = value.split(delim);
    if (items.isEmpty()) {
        QLOG_ERROR() << "GetHeaderList():" << name << "is empty";
    };
    return items;
}

// Return the name of the policy from a network reply.
QByteArray RateLimitPolicy::parseRateLimitPolicy(QNetworkReply* const reply)
{
    return parseHeader(reply, "X-Rate-Limit-Policy");
}

// Return the name(s) of the rule(s) from a network reply.
QByteArrayList RateLimitPolicy::parseRateLimitRules(QNetworkReply* const reply)
{
    return parseHeaderList(reply, "X-Rate-Limit-Rules", ',');
}

// Return a list of one or more items that define a rule's limits.
QByteArrayList RateLimitPolicy::parseRateLimit(QNetworkReply* const reply, const QByteArray& rule)
{
    return parseHeaderList(reply, "X-Rate-Limit-" + rule, ',');
}

// Return a list of one or more items that define a rule's current state.
QByteArrayList RateLimitPolicy::parseRateLimitState(QNetworkReply* const reply, const QByteArray& rule)
{
    return parseHeaderList(reply, "X-Rate-Limit-" + rule + "-State", ',');
}

// Return the date from the HTTP reply headers.
QDateTime RateLimitPolicy::parseDate(QNetworkReply* const reply)
{
    const QByteArray timestamp = utils::replace_obsolete_timezones(parseHeader(reply, "Date"));
    const QDateTime date = QDateTime::fromString(timestamp, Qt::RFC2822Date).toLocalTime();
    if (!date.isValid()) {
        QLOG_ERROR() << "invalid date parsed from" << timestamp;
    };
    return date;
}

// Return the HTTP status from the reply headers.
int RateLimitPolicy::parseStatus(QNetworkReply* const reply)
{
    return reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
}