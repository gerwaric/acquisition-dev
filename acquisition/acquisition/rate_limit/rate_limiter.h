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

#include <acquisition/oauth/oauth_token.h>
#include <acquisition/rate_limit/rate_limited_reply.h>
#include <acquisition/rate_limit/rate_limit_manager.h>
#include <acquisition/rate_limit/rate_limit_policy.h>

#include <QObject>

#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QString>
#include <QTimer>

#include <list>
#include <map>
#include <memory>
#include <optional>

class OAuthManager;

class RateLimiter : public QObject {
    Q_OBJECT

public:
    // Create a rate limiter.
    explicit RateLimiter(
        QObject* parent,
        QNetworkAccessManager& network_manager);

    // Submit a request-callback pair to the rate limiter. The caller is responsible
    // for freeing the RateLimitedReply object with deleteLater() when the completed()
    // signal has been emitted.
    RateLimitedReply* Submit(
        const QString& endpoint,
        QNetworkRequest network_request);

public slots:

    void setToken(const OAuthToken& token);

    // Used by the GUI to request a manual refresh.
    void OnUpdateRequested();

signals:
    // Emitted when one of the policy managers has signalled a policy update.
    void PolicyUpdate(const RateLimitPolicy& policy);

    // Emitted when a request has been added to a queue.
    void QueueUpdate(const QString& policy_name, int queued_requests);

    // Signal sent to the UI so the user can see what's going on.
    void Paused(int seconds, const QString& policy_name);

private slots:

    void SendStatusUpdate();

    // Received from individual policy managers.
    void OnPolicyUpdated(const RateLimitPolicy& policy);

    // Received from individual policy managers.
    void OnQueueUpdated(const QString& policy_name, int queued_requests);

    // Received from individual policy managers.
    void OnManagerPaused(const QString& policy_name, const QDateTime& until);

private:

    // Process the first request for an endpoint we haven't encountered before.
    void SetupEndpoint(
        const QString& endpoint,
        QNetworkRequest network_request,
        RateLimitedReply* reply);

    // Process the first request for an endpoint we haven't encountered before.
    void ProcessHeadResponse(
        const QString& endpoint,
        QNetworkRequest network_request,
        RateLimitedReply* reply,
        QNetworkReply* network_reply);

    // Log extra details about the HEAD request and replies
    void LogSetupReply(
        const QNetworkRequest& request,
        const QNetworkReply* reply);

    // Get or create the rate limit policy manager for the given endpoint.
    RateLimitManager& GetManager(
        const QString& endpoint,
        const QString& policy_name);

    // Reference to the Application's network access manager.
    QNetworkAccessManager& network_manager_;

    OAuthToken oauth_token_;

    QTimer update_timer_;

    std::map<const QDateTime, QString> pauses_;

    std::list<RateLimitManager> managers_;
    std::map<const QString, RateLimitManager&> manager_by_policy_;
    std::map<const QString, RateLimitManager&> manager_by_endpoint_;

};
