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

#include <acquisition/oauth/oauth_settings.h>

#include <QObject>
#include <QTimer>
#include <QtHttpServer/QHttpServer>

#include <string>

class QHttpServerRequest;
class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;

class OAuthManager : public QObject {
    Q_OBJECT
public:
    explicit OAuthManager(
        QObject* parent,
        QNetworkAccessManager& network_manager,
        const OAuthSettings& settings);
    void addAuthorization(QNetworkRequest& request) const;
public slots:
    void requestAccess();
    void requestRefresh();
    void showStatus();
signals:
    void accessGranted(const OAuthToken& token);
private:
    void createHttpServer();
    void requestAuthorization(const std::string& state, const std::string& code_challenge);
    QString receiveAuthorization(const QHttpServerRequest& request, const std::string& state);
    void requestToken(const std::string& code);
    void receiveToken(QNetworkReply* reply);
    void setRefreshTimer();

    static QString authorizationError(const QString& message);

    // Used to send network requests.
    QNetworkAccessManager& m_network_manager;

    const OAuthSettings m_settings;

    // I can't find a way to shutdown a QHttpServer once it's started
    // listening, so use a unique pointer so that we can destory the
    // server once authentication is complete, so it won't stay
    // running in the background.
    std::unique_ptr<QHttpServer> m_http_server;

    std::string m_code_verifier;
    std::string m_redirect_uri;
    QTimer m_refresh_timer;

    std::unique_ptr<const OAuthToken> m_token;

};
