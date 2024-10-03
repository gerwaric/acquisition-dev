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
#include <acquisition/oauth/oauth_token.h>

#include <acquisition/utils/utils.h>

#include <QNetworkReply>
#include <QsLog/QsLog.h>


OAuthToken::OAuthToken(const QString& json)
{
    const std::string bytes = json.toStdString();
    JS::ParseContext context(bytes);
    if (context.parseTo(*this) != JS::Error::NoError) {
        QLOG_ERROR() << "Error parsing OAuthToken from byte array:" << context.makeErrorString();
    };
}

OAuthToken::OAuthToken(QNetworkReply* const reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        QLOG_ERROR() << "Cannot construct the OAuth token due a network error:" << reply->errorString();
        return;
    };
    const QByteArray token_bytes = reply->readAll();
    JS::ParseContext context(token_bytes);
    JS::Error result = context.parseTo(*this);
    if (result != JS::Error::NoError) {
        QLOG_ERROR() << "Error parsing the OAuthToken from a network reply:" << context.makeErrorString();
        return;
    };
    if (!reply->hasRawHeader("Date")) {
        QLOG_ERROR() << "Error setting the OAuthToken birthdy: 'Date' is missing from the network reply.";
        return;
    };
    const QByteArray date_bytes = reply->rawHeader("Date");
    birthday = utils::parseRFC2822(date_bytes).toLocalTime();
    expiration = birthday.value().addSecs(expires_in);
    QLOG_TRACE() << "OAuthToken birthday is" << birthday.value();
    QLOG_TRACE() << "OAuthToken expiration is" << expiration.value();
}

void OAuthToken::authorize(QNetworkRequest& request)
{
    if (!isValid()) {
        QLOG_ERROR() << "Cannot set authentication: OAuthToken is invalid";
        return;
    };
    request.setRawHeader("Authorization", "Bearer " + access_token);
}

bool OAuthToken::isValid() const
{
    return (!access_token.isEmpty())
        && (expiration.has_value())
        && (expiration.value().isValid())
        && (expiration.value() > QDateTime::currentDateTime());
}
