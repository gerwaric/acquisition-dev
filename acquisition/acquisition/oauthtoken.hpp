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

#include <QDateTime>
#include <QString>

class OAuthToken {
public:
    OAuthToken();

    QString access_token() const { return access_token_; };
    int expires_in() const { return expires_in_; };
    QString scope() const { return scope_; };
    QString username() const { return username_; };
    QString sub() const { return sub_; };
    QString refresh_token() const { return refresh_token_; };

private:

    QString access_token_;
    long long int expires_in_;
    QString token_type_;
    QString scope_;
    QString username_;
    QString sub_;
    QString refresh_token_;

    QDateTime birthday_;
    QDateTime expiration_;
};