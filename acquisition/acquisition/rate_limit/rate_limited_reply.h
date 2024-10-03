#pragma once

#include <QObject>

class QNetworkReply;

// This is the object returned to the end-user of the rate limiter.
// When the underlying network request is finished, the complete
// signal will be issued so that the caller can use a slot to
// process the reply.
class RateLimitedReply : public QObject {
    Q_OBJECT
signals:
    void finished(QNetworkReply* reply);
    void errorOccured(QNetworkReply* reply);
};