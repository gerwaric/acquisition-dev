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
    : hits_(-1)
    , period_(-1)
    , restriction_(-1)
{
    const QByteArrayList parts = header_fragment.split(':');
    hits_ = parts[0].toInt();
    period_ = parts[1].toInt();
    restriction_ = parts[2].toInt();
}

//=========================================================================================
// RateLimitPolicy::Item
//=========================================================================================

RateLimitPolicy::Item::Item(const QByteArray& limit_fragment, const QByteArray& state_fragment)
    : limit_(limit_fragment)
    , state_(state_fragment)
{
    // Determine the status of this item.
    if (state_.period() != limit_.period()) {
        status_ = RateLimitPolicy::Status::Invalid;
    } else if (state_.hits() > limit_.hits()) {
        status_ = RateLimitPolicy::Status::Violation;
    } else if (state_.hits() == limit_.hits()) {
        status_ = RateLimitPolicy::Status::Borderline;
    } else {
        status_ = RateLimitPolicy::Status::Ok;
    };
}

void RateLimitPolicy::Item::Check(const RateLimitPolicy::Item& other, const QString& prefix) const
{
    if (limit_.hits() != other.limit_.hits()) {
        QLOG_WARN() << prefix << "limit.hits changed"
            << "from" << limit_.hits()
            << "to" << other.limit_.hits();
    };
    if (limit_.period() != other.limit_.period()) {
        QLOG_WARN() << prefix << "limit.period changed"
            << "from" << limit_.period()
            << "to" << other.limit_.period();
    };
    if (limit_.restriction() != other.limit_.restriction()) {
        QLOG_WARN() << prefix << "limit.restriction changed"
            << "from" << limit_.restriction()
            << "to" << other.limit_.restriction();
    };
}

QDateTime RateLimitPolicy::Item::GetNextSafeSend(const boost::circular_buffer<QDateTime>& history) const
{
    QLOG_TRACE() << "RateLimit::RuleItem::GetNextSafeSend() entered";

    const QDateTime now = QDateTime::currentDateTime().toLocalTime();

    // We can send immediately if we have not bumped up against a rate limit.
    if (state_.hits() < limit_.hits()) {
        return now;
    };

    // Determine how far back into the history we can look.
    const size_t n = (limit_.hits() > history.size()) ? history.size() : limit_.hits();

    // Start with the timestamp of the earliest known 
    // reply relevant to this limitation.
    const QDateTime earliest = (n < 1) ? now : history[n - 1];

    const QDateTime next_send = earliest.addSecs(limit_.period());

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

    const int current_hits = state_.hits();
    const int max_hits = limit_.hits();
    const int period_length = limit_.period();
    const int restriction = limit_.restriction();

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
    : name_(rule_name)
    , maximum_hits_(-1)
    , status_(RateLimitPolicy::Status::Unknown)
{
    QLOG_TRACE() << "RateLimit::PolicyRule::PolicyRule() entered";
    const QByteArrayList limit_fragments = parseRateLimit(reply, rule_name);
    const QByteArrayList state_fragments = parseRateLimitState(reply, rule_name);
    const int item_count = limit_fragments.size();
    if (state_fragments.size() != limit_fragments.size()) {
        QLOG_ERROR() << "Invalid data for policy role.";
    };
    items_.reserve(item_count);
    for (int j = 0; j < item_count; ++j) {

        // Create a new rule item from the next pair of fragments.
        const auto& item = items_.emplace_back(limit_fragments[j], state_fragments[j]);

        // Keep track of the max hits, max rate, and overall status.
        if (maximum_hits_ < item.limit().hits()) {
            maximum_hits_ = item.limit().hits();
        };
        if (status_ < item.status()) {
            status_ = item.status();
        };
    };
}

void RateLimitPolicy::Rule::Check(const RateLimitPolicy::Rule& other, const QString& prefix) const
{
    QLOG_TRACE() << "RateLimit::PolicyRule::Check() entered";

    // Check the rule name
    if (name_ != other.name_) {
        QLOG_WARN() << prefix << "rule name changed"
            << "from" << name_ << "to" << other.name_;
    };

    // Check the number of items in this rule
    if (items_.size() != other.items_.size()) {

        // The number of items changed
        QLOG_WARN() << prefix << "rule" << name_ << "went"
            << "from" << items_.size() << "items"
            << "to" << other.items_.size() << "items";

    } else {

        // Check each item
        for (int i = 0; i < items_.size(); ++i) {
            const QString item_prefix = QString("%1 item #%2").arg(prefix, QString::number(i));
            const auto& old_item = items_[i];
            const auto& new_item = other.items_[i];
            old_item.Check(new_item, item_prefix);
        };

    };
}

//=========================================================================================
// RateLimitPolicy
//=========================================================================================

RateLimitPolicy::RateLimitPolicy(QNetworkReply* const reply)
    : maximum_hits_(0)
    , status_(RateLimitPolicy::Status::Unknown)
{
    QLOG_TRACE() << "RateLimit::Policy::Policy() entered";
    const QByteArrayList rule_names = parseRateLimitRules(reply);

    // Parse the name of the rate limit policy and all the rules for this reply.
    name_ = parseRateLimitPolicy(reply);
    rules_.reserve(rule_names.size());

    // Iterate over all the rule names expected.
    for (const auto& rule_name : rule_names) {

        // Create a new rule and add it to the list.
        const auto& rule = rules_.emplace_back(rule_name, reply);

        // Check the status of this rule..
        if (rule.status() >= RateLimitPolicy::Status::Violation) {
            QLOG_ERROR() << QString("Rate limit policy '%1:%2' is %4)").arg(
                name_, rule.name(), QVariant::fromValue(rule.status()).toString());
        } else if (rule.status() == RateLimitPolicy::Status::Borderline) {
            QLOG_DEBUG() << QString("Rate limit policy '%1:%2' is BORDERLINE").arg(
                name_, rule.name());
        };

        // Update metrics for this rule.
        if (maximum_hits_ < rule.maximum_hits()) {
            maximum_hits_ = rule.maximum_hits();
        };
        if (status_ < rule.status()) {
            status_ = rule.status();
        };
    };
}

void RateLimitPolicy::RateLimitPolicy::Check(const RateLimitPolicy& other) const
{
    QLOG_TRACE() << "RateLimit::Policy::Check() entered";

    // Check the policy name
    if (name_ != other.name_) {
        QLOG_WARN() << "The rate limit policy name change from" << name_ << "to" << other.name_;
    };

    // Check the number of rules
    if (rules_.size() != other.rules_.size()) {

        // The number of rules changed
        QLOG_WARN() << "The rate limit policy" << name_
            << "had" << rules_.size() << "rules,"
            << "but now has" << other.rules_.size();

    } else {

        // The number of rules is the same, so check each one
        for (int i = 0; i < rules_.size(); ++i) {
            const QString prefix = QString("Rate limit policy %1, rule #%2:").arg(name_, QString::number(i));
            const auto& old_rule = rules_[i];
            const auto& new_rule = other.rules_[i];
            old_rule.Check(new_rule, prefix);
        };

    };
}

QDateTime RateLimitPolicy::RateLimitPolicy::GetNextSafeSend(const boost::circular_buffer<QDateTime>& history)
{
    QLOG_TRACE() << "RateLimit::Policy::GetNextSafeSend() entered";

    QDateTime next_send = QDateTime::currentDateTime().toLocalTime();
    for (const auto& rule : rules_) {
        for (const auto& item : rule.items()) {
            const QDateTime t = item.GetNextSafeSend(history);
            QLOG_TRACE() << "RateLimit::Policy::GetNextSafeSend()"
                << name_ << rule.name() << "t =" << t.toString()
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
        for (const auto& rule : rules_) {
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