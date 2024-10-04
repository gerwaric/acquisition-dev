/*
    Acquisition is a stash management program for Path of Exile

    Copyright(C) 2024 Gerwaric

    This program is free software : you can redistribute it and /or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.If not, see < https://www.gnu.org/licenses/>.
*/
#pragma once

#include <boost/circular_buffer.hpp>

#include <QObject>

#include <QByteArray>
#include <QByteArrayList>
#include <QDateTime>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>

//--------------------------------------------------------------------------
// Introduction to GGG's API Rate Limits
//--------------------------------------------------------------------------
// 
// As of August 2023, GGG has implemented "new" rate limit policies that are
// incompatible with how acquisition used to throttle requests. There was no
// obvious way to fix the network request code used by the ItemsManagerWorker.
// Instead, what I've done instead is create a wrapper that accepts network
// requests from the ItemsManagerWorker and hidden the implementation details.
// 
// See https://www.pathofexile.com/developer/docs for more details on the rate
// limit information that is included in the HTTP headers of the network reply
// to every API request.
//
// Those rate limit policies can change at any time, from one network
// call to the next. For example, GGG might decide to temporarily tighten rate
// limitations around a league launch.
// 
// This means that any solution that hard-codes things like delays will
// eventually break.
// 
// What this means for us is that we have to check the reply to every
// network request for the details of rate-limitation policy that were applied.
// 
// Each policy can have mulitple rules that apply to it. Each rule
// can have multiple elments. Each element defines the specific number of 
// API hits that are allowed, the period within which those hits are measured,
// and the timeout restriction if that item is violated.
// 
// The RateLimiter class defined below takes care of all of that.
// 
// Specifically, there are a number of helper functions and classes in
// ratelimit.cpp that are used to keep track of everything, limit network
// request as little as possible, and do all of that transparently, so that
// ItemsManagerWorker or the rest of the application don't need to be changed.
// 
// More specifically, the RateLimiter::Submit method takes a network request
// and a callback function. See the code in itemsmanagerworker.cpp for
// examples of how this is used.
// 
// Submitted requests are sent out serially. No request is sent until a reply
// to the previous request is received. If a rate limit violation is detected,
// a request will be resent after the required delay. This allows the wrapper
// to monitor the exact state of all the rate-limit policies and inject delays
// as necessary to avoid violating rate limit policies.
// 
// This approach also alows us to forgo hardcoding anything about the rate
// limits in the source code. Instead, everything about the rate limits is
// take from the relevant HTTP headers in each network reply.
//
// WARNINGS:
//
//      This code may also break where there are other sources of rate-limited
//      API requests, e.g. if someone is using two tools on the same computer
//      with the same account.

//=========================================================================================
// Next, declarations for the classes that represent a rate-limit policy
//=========================================================================================
//
// Each API response has a rate-limit policy that applies to it.
// Those responses are present in the HTTP reply headers. Here's
// how they are concieved, briefly:
// 
//  Every endpoint only has one applicable policy.
//  Different endpoints may share the same rate limit policy.
// 
//  A policy has a name.
//  A policy has one or more rules.
//  A policy applies to one or more endpoints.
// 
//  Each rule has a name.
//  Each rule has one or more items.
// 
//  Each item has data that defines one set of limits.
//  Each item has data on the state of those limts.
//
// For any request against a rate-limited endpoint, only one policy applies, but
// all of limitations for each item of every rule within that policy are checked.

class RateLimitPolicy : public QObject {
    Q_OBJECT

public:

    static QByteArray parseHeader(QNetworkReply* const reply, const QByteArray& name);
    static QByteArrayList parseHeaderList(QNetworkReply* const reply, const QByteArray& name, const char delim);
    static QByteArray parseRateLimitPolicy(QNetworkReply* const reply);
    static QByteArrayList parseRateLimitRules(QNetworkReply* const reply);
    static QByteArrayList parseRateLimit(QNetworkReply* const reply, const QByteArray& rule);
    static QByteArrayList parseRateLimitState(QNetworkReply* const reply, const QByteArray& rule);
    static QDateTime parseDate(QNetworkReply* const reply);
    static int parseStatus(QNetworkReply* const reply);

    enum class Status { Unknown, Ok, Borderline, Violation, Invalid };
    Q_ENUM(Status)

    class Element {
    public:
        Element(const QByteArray& header_fragment);
        int hits{ -1 };
        int period{ -1 };
        int restriction{ -1 };
    };

    class Item {
    public:
        Item(const QByteArray& limit_fragment, const QByteArray& state_fragment);
        inline const Element& limit() const { return m_limit; };
        inline const Element& state() const { return m_state; };
        inline int maximum_hits() const { return m_maximum_hits; };
        inline Status status() const { return m_status; };
        void Check(const Item& other, const QString& prefix) const;
        QDateTime GetNextSafeSend(const boost::circular_buffer<QDateTime>& history) const;
        int EstimateDuration(int request_count, int minimum_delay_msec) const;
    private:
        Element m_limit;
        Element m_state;
        int m_maximum_hits{ -1 };
        Status m_status{ Status::Unknown };
    };

    class Rule {
    public:
        Rule(const QByteArray& rule_name, QNetworkReply* const reply);
        inline const QString& name() const { return m_name; };
        inline const std::vector<Item>& items() const { return m_items; };
        inline const int maximum_hits() const { return m_maximum_hits; };
        inline const Status status() const { return m_status; };
        void Check(const Rule& other, const QString& prefix) const;
    private:
        QString m_name;
        std::vector<Item> m_items;
        int m_maximum_hits{ -1 };
        Status m_status{ Status::Unknown };
    };

    RateLimitPolicy(QNetworkReply* const reply);
    inline const QString& name() const { return m_name; };
    inline const std::vector<Rule>& rules() const { return m_rules; };
    inline const int maximum_hits() const { return m_maximum_hits; };
    inline const Status status() const { return m_status; };

    void Check(const RateLimitPolicy& other) const;
    QDateTime GetNextSafeSend(const boost::circular_buffer<QDateTime>& history);
    QDateTime EstimateDuration(int request_count, int minimum_delay_msec) const;

private:
    QString m_name;
    std::vector<Rule> m_rules;
    int m_maximum_hits{ -1 };
    Status m_status;
};
