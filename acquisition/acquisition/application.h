#pragma once

#include <acquisition/api_types/character.h>
#include <acquisition/api_types/league.h>
#include <acquisition/api_types/stash_tab.h>

#include <acquisition/endpoint_manager.h>
#include <acquisition/items_manager.h>
#include <acquisition/main_window.h>
#include <acquisition/oauth/oauth_manager.h>
#include <acquisition/oauth/oauth_settings.h>
#include <acquisition/rate_limit/rate_limiter.h>
#include <acquisition/settings.h>

#include <QObject>

#include <QNetworkAccessManager>
#include <QString>
#include <QStringList>

class Application : public QObject
{
public:
    Application();

public slots:
    //void onAuthenticationComplete();
    void receiveLeagueList(std::vector<poe_api::League> all_leagues);

private:
    
    void authenticate();

    static const OAuthSettings oauth_settings;

    Settings settings;
    QNetworkAccessManager network_manager;
    OAuthManager oauth_manager;
    RateLimiter rate_limiter;
    EndpointManager endpoint_manager;
    ItemsManager items_manager;
    MainWindow main_window;

    QStringList leagues_;
    QString current_league_;

};
