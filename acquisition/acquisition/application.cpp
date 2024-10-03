#include <acquisition/application.h>

#include <acquisition/authentication_dialog.h>

#include <QsLog/QsLog.h>

#include <QApplication>

const OAuthSettings Application::oauth_settings = {
    "https://www.pathofexile.com/oauth/authorize",
    "https://www.pathofexile.com/oauth/token",
    "acquisition",
    "account:leagues account:stashes account:characters",
    "http://127.0.0.1",
    "/auth/path-of-exile" };

Application::Application()
    : network_manager(this)
    , oauth_manager(this, network_manager, oauth_settings)
    , rate_limiter(this, network_manager)
    , endpoint_manager(this, rate_limiter)
{
    // Authentication
    connect(&oauth_manager, &OAuthManager::accessGranted, &rate_limiter, &RateLimiter::setToken);

    // Menu Actions
    connect(&main_window, &MainWindow::refreshCharacterList, &endpoint_manager, &EndpointManager::listCharacters);
    connect(&main_window, &MainWindow::refreshStashList, &endpoint_manager,
        [this]() {
            endpoint_manager.listStashes("Standard");
        });

    // Refreshing Characters and Stash Tabs
    connect(&endpoint_manager, &EndpointManager::leagueListReceived, &main_window, &MainWindow::receiveLeagueList);
    connect(&endpoint_manager, &EndpointManager::characterListReceived, &items_manager, &ItemsManager::receiveCharacterList);
    connect(&endpoint_manager, &EndpointManager::characterReceived, &items_manager, &ItemsManager::receiveCharacter);
    connect(&endpoint_manager, &EndpointManager::stashListReceived, &items_manager, &ItemsManager::receiveStashList);
    connect(&endpoint_manager, &EndpointManager::stashReceived, &items_manager, &ItemsManager::receiveStash);

    main_window.show();

    authenticate();
}

void Application::authenticate()
{
    AuthenticationDialog* login = new AuthenticationDialog();
    connect(login, &AuthenticationDialog::requested, &oauth_manager, &OAuthManager::requestAccess);
    connect(&oauth_manager, &OAuthManager::accessGranted, login, &AuthenticationDialog::setToken);
    login->exec();
    
    endpoint_manager.getLeagues();

    main_window.activateWindow();
    main_window.raise();
}

