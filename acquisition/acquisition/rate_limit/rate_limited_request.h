#pragma once

#include <acquisition/rate_limit/rate_limited_reply.h>

#include <QNetworkRequest>
#include <QString>

// Represents a single rate-limited request.
struct RateLimitedRequest {

    // Construct a new rate-limited request.
    RateLimitedRequest(
        const QString& endpoint_,
        const QNetworkRequest& network_request_,
        RateLimitedReply* reply_)
        : id(++request_count)
        , endpoint(endpoint_)
        , network_request(network_request_)
        , reply(reply_) {}

    // Unique identified for each request, even through different requests can be
    // routed to different policy managers based on different endpoints.
    const unsigned long id;

    const QString endpoint;

    QNetworkRequest network_request;

    std::unique_ptr<RateLimitedReply> reply;

private:

    // Total number of requests that have every been constructed.
    static unsigned long request_count;
};
