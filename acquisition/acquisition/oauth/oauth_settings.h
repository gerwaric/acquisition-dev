#pragma once

#include <QString>

struct OAuthSettings {
    QString authorization_url;
    QString token_url;
    QString client_id;
    QString client_scope;
    QString redirect_url;
    QString redirect_path;
};
