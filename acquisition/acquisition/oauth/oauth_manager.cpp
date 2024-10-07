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
#include <acquisition/oauth/oauth_manager.h>

#include <acquisition/constants.h>

#include <json_struct/json_struct.h>
#include <QsLog/QsLog.h>

#include <QByteArray>
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QtHttpServer/QHttpServer>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QString>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QUuid>

// Refresh a token an hour before it's due to expire.
constexpr int EXPIRATION_BUFFER_SECS = 3600;

OAuthManager::OAuthManager(
    QObject* parent,
    QNetworkAccessManager& network_manager,
    const OAuthSettings& settings)
    : QObject(parent)
    , m_network_manager(network_manager)
    , m_settings(settings)
    , m_refresh_timer(this)
{
    QLOG_TRACE() << "OAuthManager::OAuthManager() entered";

    // Configure the refresh timer.
    m_refresh_timer.setSingleShot(true);
    connect(&m_refresh_timer, &QTimer::timeout, this, &OAuthManager::requestRefresh);
}

void OAuthManager::addAuthorization(QNetworkRequest& request) const
{
    QLOG_TRACE() << "OAuthManager::setAuthorization() entered";
    if (m_token->access_token.isEmpty()) {
        QLOG_ERROR() << "Cannot set OAuth authorization header: there is no token.";
        return;
    };
    if (m_token->expiration.value_or(QDateTime()) <= QDateTime::currentDateTime()) {
        QLOG_ERROR() << "Cannot set OAuth authorization header: the token has expired.";
        return;
    };
    const QString bearer = "Bearer " + m_token->access_token;
    request.setRawHeader("Authorization", bearer.toUtf8());
}

void OAuthManager::setToken(const OAuthToken& token)
{
    if (!token.isValid()) {
        QLOG_ERROR() << "OAuthManager: token is invalid";
        return;
    };
    m_token = std::make_unique<OAuthToken>(token);
    emit accessGranted(token);
}

void OAuthManager::setRefreshTimer()
{
    QLOG_TRACE() << "OAuthManager::setRefreshTimer() entered";
    const QDateTime refresh_date = m_token->expiration.value_or(QDateTime()).addSecs(-EXPIRATION_BUFFER_SECS);
    const unsigned long interval = QDateTime::currentDateTime().msecsTo(refresh_date);
    m_refresh_timer.setInterval(interval);
    m_refresh_timer.start();
    QLOG_INFO() << "OAuth: refreshing token at" << refresh_date.toString();
}

void OAuthManager::requestAccess()
{
    QLOG_TRACE() << "OAuthManager::setAccess() entered";

    // Build the state.
    const auto state_data = (
        QUuid::createUuid().toString(QUuid::WithoutBraces) +
        QUuid::createUuid().toString(QUuid::WithoutBraces)).toLatin1(); // 43 <= length <= 128
    const auto state_hash = QCryptographicHash::hash(state_data, QCryptographicHash::Sha256);
    const auto state = state_hash.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

    // Create the code challenge.
    m_code_verifier = (
        QUuid::createUuid().toString(QUuid::WithoutBraces) +
        QUuid::createUuid().toString(QUuid::WithoutBraces)).toLatin1(); // 43 <= length <= 128
    const auto code_hash = QCryptographicHash::hash(m_code_verifier, QCryptographicHash::Sha256);
    const auto code_challenge = code_hash.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

    // Setup an http server so we know what port to listen on.
    createHttpServer();
    if (m_http_server == nullptr) {
        QLOG_ERROR() << "Unable to create the http server for OAuth authorization.";
        return;
    };

    // Get the port for the callback.
    const auto port = m_http_server->listen();
    if (port == 0) {
        QLOG_ERROR() << "Unable to bind the http server for OAuth authorization.";
        return;
    };

    // Build the callback URI.
    QUrl url;
    url.setUrl(m_settings.redirect_url);
    url.setPort(port);
    url.setPath(m_settings.redirect_path);
    m_redirect_uri = url.toString().toStdString();

    // Make the authorization request.
    requestAuthorization(state.toStdString(), code_challenge.toStdString());
}

void OAuthManager::createHttpServer()
{
    QLOG_TRACE() << "OAuthManager::createHttpServer() entered";

    // Create a new HTTP server.
    m_http_server = std::make_unique<QHttpServer>(this);

    // Tell the server to ignore favicon requests, even though these
    // should be disabled based on the HTML we are returning.
    m_http_server->route("/favicon.ico",
        [](const QHttpServerRequest& request, QHttpServerResponder&& responder) {
            Q_UNUSED(request);
            Q_UNUSED(responder);
            QLOG_TRACE() << "OAuth: ignoring favicon.ico request";
        });

    // Capture all unhandled requests for debugging.
    m_http_server->setMissingHandler(
        [](const QHttpServerRequest& request, QHttpServerResponder&& responder) {
            Q_UNUSED(responder);
            QLOG_TRACE() << "OAuth: unhandled request:" << request.url().toString();
        });
}

void OAuthManager::requestAuthorization(const std::string& state, const std::string& code_challenge)
{
    QLOG_TRACE() << "OAuthManager::requestAuthorization() entered";

    // Create the authorization query.
    const QUrlQuery query({
        {"client_id", m_settings.client_id},
        {"response_type", "code"},
        {"scope", m_settings.client_scope},
        {"state", QString::fromStdString(state)},
        {"redirect_uri", QString::fromStdString(m_redirect_uri)},
        {"code_challenge", QString::fromStdString(code_challenge)},
        {"code_challenge_method", "S256"}
        });

    // Prepare the url.
    QUrl url;
    url.setUrl(m_settings.authorization_url);
    url.setQuery(query);

    // Make sure the state is passed to the function that receives the authorization response.
    m_http_server->route(m_settings.redirect_path,
        [=](const QHttpServerRequest& request) {
            return receiveAuthorization(request, state);
        });

    // Use the user's browser to open the authorization url.
    QLOG_TRACE() << "OAuthManager::requestAuthorization() opening url";
    QDesktopServices::openUrl(url);
}

QString OAuthManager::authorizationError(const QString& message)
{
    QLOG_ERROR() << "OAuth: authorization error:" << message;
    const QString ERROR_HTML = QString(R"html(
        <html>
			<head>
				<link rel="icon" href="data:, ">
				<title>OAuth Authorization Error</title>
			</head>
			<body>
				<p>%2</p>
			</body>
		</html>)html").simplified();
    return ERROR_HTML.arg(message);
}

QString OAuthManager::receiveAuthorization(const QHttpServerRequest& request, const std::string& state)
{
    QLOG_TRACE() << "OAuthManager::receiveAuthorization() entered";

    // Shut the server down now that an access token response has been received.
    // Don't do it immediately in case the browser wants to request a favicon, even
    // though I've tried to disable that by including icon links in HTML.
    QTimer::singleShot(1000, this,
        [=]() {
            m_http_server = nullptr;
        });

    const QUrlQuery& query = request.query();

    // Check for errors.
    if (query.hasQueryItem("error")) {
        QString error_message = query.queryItemValue("error");
        const QString error_desription = query.queryItemValue("error_description");
        const QString error_uri = query.queryItemValue("error_uri");
        if (error_desription.isEmpty() == false) {
            error_message += " : " + error_desription;
        };
        if (error_uri.isEmpty() == false) {
            error_message += " : " + error_uri;
        };
        return authorizationError(error_message);
    };

    const QString auth_code = query.queryItemValue("code");
    const QString auth_state = query.queryItemValue("state");

    const QString SUCCESS_HTML = QString(R"html(
		<html>
			<head>
				<link rel="icon" href="data:, ">
				<title>Acquisition</title>
				<style>
					html, body, .container { height: 75%; }
					.container { display: flex; align-items: center; justify-content: center; }
				</style>
			</head>
			<body>
				<h1 class="container">Acquisition has been authorized.<br>You may close this page.</h1>
			</body>
		</html>")html").simplified();

    const QString ERROR_HTML = QString(R"html(
        <html>
		    <head>
			    <link rel="icon" href="data:, ">
			    <title>OAuth Authorization Error</title>
		    </head>
		    <body>
			    <p>%2</p>
		    </body>
	    </html>)html").simplified();

    // Make sure the code and state look valid.
    QString error_message;
    if (auth_code.isEmpty()) {
        error_message = "Invalid authorization response: 'code' is missing.";
    } else if (auth_state.isEmpty()) {
        error_message = "Invalid authorization response: 'state' is missing.";
    } else if (auth_state != QString::fromStdString(state)) {
        error_message = "Invalid authorization repsonse: 'state' is invalid!";
    } else {
        // Use the code to request an access token.
        requestToken(auth_code.toStdString());
        return SUCCESS_HTML;
    };

    QLOG_ERROR() << "OAuth: authorization error:" << error_message;
    return ERROR_HTML.arg(error_message);
};

void OAuthManager::requestToken(const std::string& code)
{
    QLOG_TRACE() << "OAuthManager::requestToken() entered";

    QNetworkRequest request;
    request.setUrl(QUrl(m_settings.token_url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setHeader(QNetworkRequest::UserAgentHeader, USER_AGENT);

    const QUrlQuery query({
        {"client_id", m_settings.client_id},
        {"grant_type", "authorization_code"},
        {"code", QString::fromStdString(code)},
        {"redirect_uri", QString::fromStdString(m_redirect_uri)},
        {"scope", m_settings.client_scope},
        {"code_verifier", QString::fromStdString(m_code_verifier)} });
    const QByteArray data = query.toString(QUrl::FullyEncoded).toUtf8();
    QNetworkReply* reply = m_network_manager.post(request, data);

    connect(reply, &QNetworkReply::finished, this,
        [=]() {
            receiveToken(reply);
            reply->deleteLater();
        });

    connect(reply, &QNetworkReply::errorOccurred, this,
        [=]() {
            QLOG_ERROR() << "Error requesting OAuth access token:" << reply->errorString();
            reply->deleteLater();
        });
}

void OAuthManager::receiveToken(QNetworkReply* reply)
{
    QLOG_TRACE() << "OAuthManager::receiveToken() entered";

    if (reply->error() != QNetworkReply::NoError) {
        QLOG_ERROR() << "OAuth: http error"
            << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) << ":"
            << reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute);
        return;
    };

    // Parse the token and emit it.
    QLOG_TRACE() << "OAuthManager::receiveToken() parsing OAuth access token";
    m_token = std::make_unique<OAuthToken>(reply);

    emit accessGranted(*m_token);

    // Setup the refresh timer.
    setRefreshTimer();

    // Make sure to dispose of the reply.
    reply->deleteLater();
}

void OAuthManager::requestRefresh()
{
    QLOG_TRACE() << "OAuthManager::requestRefresh() entered";

    // Update the user.
    static std::unique_ptr<QMessageBox> msgBox = nullptr;
    if (!msgBox) {
        msgBox = std::make_unique<QMessageBox>();
        msgBox->setWindowTitle(APP_NAME " - OAuth Token Refresh");
        msgBox->setModal(false);
    };

    msgBox->setText("Your OAuth token is being refreshed.");

    // Setup the refresh query.
    const QUrlQuery query({
        {"client_id", m_settings.client_id},
        {"grant_type", "refresh_token"},
        {"refresh_token", m_token->refresh_token} });
    const QByteArray data = query.toString(QUrl::FullyEncoded).toUtf8();

    // Make and submit the POST request.
    QNetworkRequest request;
    request.setUrl(QUrl(m_settings.token_url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setHeader(QNetworkRequest::UserAgentHeader, USER_AGENT);
    QNetworkReply* reply = m_network_manager.post(request, data);

    connect(reply, &QNetworkReply::finished, this,
        [=]() {
            // Update the user again after the token has been received.
            receiveToken(reply);
            const QStringList message = {
                "Your OAuth token was refreshed on " + m_token->birthday.value_or(QDateTime()).toString(),
                "",
                "The new token expires on " + m_token->expiration.value_or(QDateTime()).toString(),
            };
            msgBox->setText(message.join("\n"));
            msgBox->show();
            msgBox->raise();
            reply->deleteLater();
        });

    connect(reply, &QNetworkReply::errorOccurred, this,
        [=]() {
            // Let the user know if there was an error.
            const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            const auto reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
            QLOG_ERROR() << "OAuthManager::requestRefresh() network error:" << reason;
            const QStringList message = {
                "OAuth refresh failed: " + reply->errorString(),
                "",
                "HTTP status " + QString::number(status) + " (" + reason + ")"
            };
            msgBox->setText(message.join("\n"));
            msgBox->show();
            msgBox->raise();
            reply->deleteLater();
        });
}

void OAuthManager::showStatus()
{
    QLOG_TRACE() << "OAuthManager::showStatus() entered";

    static std::unique_ptr<QMessageBox> msgBox = nullptr;
    if (!msgBox) {
        msgBox = std::make_unique<QMessageBox>();
        msgBox->setWindowTitle("OAuth Status - " APP_NAME " - OAuth Token Status");
        msgBox->setModal(false);
    };

    if (m_token->isValid()) {
        const std::string json = JS::serializeStruct(m_token);
        const QDateTime now = QDateTime::currentDateTime();
        const QDateTime refresh_time = now.addMSecs(m_refresh_timer.remainingTime());
        const QString refresh_timestamp = refresh_time.toString("MMM d 'at' h:m ap");
        const QStringList message = {
            "Your current OAuth token:",
            "",
            QString::fromStdString(json),
            "",
            "This token will be automatically refreshed on " + refresh_timestamp
        };
        msgBox->setText(message.join("\n"));
    } else {
        msgBox->setText("No valid token. You are not authenticated.");
    }
    msgBox->show();
    msgBox->raise();
}
