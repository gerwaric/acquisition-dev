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

#include <json_struct/json_struct.h>

#include <acquisition/utils/json.h>

#include <QDateTime>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>

#include <optional>

class OAuthToken {

public:

    OAuthToken() = default;
    OAuthToken(const QString& json);
    OAuthToken(QNetworkReply* const reply);

    bool isValid() const;

    void authorize(QNetworkRequest& request);

    QByteArray access_token;
    long long int expires_in{ -1 };
    QByteArray token_type;
    QByteArray scope;
    QByteArray sub;
    QByteArray username;
    QByteArray refresh_token;

    std::optional<QDateTime> birthday;
    std::optional<QDateTime> expiration;

    JS_OBJ(access_token, expires_in, token_type, scope, sub,
        username, refresh_token, birthday, expiration);

};
