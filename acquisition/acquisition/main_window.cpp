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
#include <acquisition/main_window.h>

#include <acquisition/authentication_dialog.h>
#include <acquisition/items_manager.h>
#include <acquisition/rate_limit/rate_limit_status_dialog.h>
#include <acquisition/tree_model.h>

#include <QsLog/QsLog.h>

#include <QComboBox>
#include <QMenuBar>
#include <QPushButton>
#include <QStatusBar>
#include <QWidget>

#include <QDockWidget>

const OAuthSettings MainWindow::s_oauth_settings = {
    "https://www.pathofexile.com/oauth/authorize",
    "https://www.pathofexile.com/oauth/token",
    "acquisition",
    "account:leagues account:stashes account:characters",
    "http://127.0.0.1",
    "/auth/path-of-exile" };

MainWindow::MainWindow(const QString& data_directory)
    : QMainWindow(nullptr)
    , m_network_manager(this)
    , m_settings(this, data_directory)
    , m_user_data(this)
    , m_league_data(this)
    , m_oauth_manager(this, m_network_manager, s_oauth_settings)
    , m_rate_limiter(this, m_network_manager)
    , m_endpoint_manager(this, m_rate_limiter)
{
    setupUserInterface();
    connectSlots();

    m_settings.sendSignals();

    show();
    authenticate();
}

void MainWindow::setupUserInterface()
{
    setMenuBar(new QMenuBar);
    setStatusBar(new QStatusBar);
    setCentralWidget(new QWidget);

    m_status_label.setText("Status");
    statusBar()->addWidget(&m_status_label);

    m_league_menu.setTitle("League");
    m_league_menu.setEnabled(false);
    menuBar()->addMenu(&m_league_menu);

    m_refresh_menu.setTitle("Refresh");
    m_refresh_menu.addAction("Update Everything", this, &MainWindow::refreshAll);
    m_refresh_menu.addSeparator();
    m_refresh_menu.addAction("Refresh Settings", this, []() {});
    m_refresh_menu.setEnabled(false);
    menuBar()->addMenu(&m_refresh_menu);

    RateLimitStatusDialog* rate_status = new RateLimitStatusDialog(m_rate_limiter);

    QPushButton* button = new QPushButton;
    button->setText("Rate Limit Status");
    connect(button, &QPushButton::clicked, rate_status, &RateLimitStatusDialog::show);
    connect(&m_rate_limiter, &RateLimiter::Paused, this,
        [button](int pause) {
            if (pause > 0) {
                button->setText("Rate limited for " + QString::number(pause) + " seconds");
                button->setStyleSheet("font-weight: bold; color: red");
            } else if (pause == 0) {
                button->setText("Rate limiting is OFF");
                button->setStyleSheet("");
            } else {
                button->setText("ERROR: pause is " + QString::number(pause));
                button->setStyleSheet("");
            };
        });
    statusBar()->addPermanentWidget(button);

    m_tree_view.setHeaderHidden(false);
    m_tree_view.setModel(&m_tree_model);
    m_tree_view.setUniformRowHeights(true);

    QDockWidget dock_left;

    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(&m_tree_view);
    centralWidget()->setLayout(layout);
}

void MainWindow::connectSlots()
{
    // Authentication
    connect(&m_oauth_manager, &OAuthManager::accessGranted, &m_rate_limiter, &RateLimiter::setToken);
    connect(&m_oauth_manager, &OAuthManager::accessGranted, this,
        [&](const OAuthToken& token) {
            m_settings.setUsername(token.username);
        });

    connect(&m_settings, &Settings::usernameChanged, &m_user_data, &UserDatabase::setUsername);
    connect(&m_settings, &Settings::usernameChanged, &m_league_data, &LeagueDatabase::setUsername);
    connect(&m_settings, &Settings::leagueChanged, &m_league_data, &LeagueDatabase::setLeague);

    // Refreshing Characters and Stash Tabs
    connect(&m_endpoint_manager, &EndpointManager::leagueListReceived, this, &MainWindow::receiveLeagueList);
    connect(&m_endpoint_manager, &EndpointManager::characterListReceived, this, &MainWindow::receiveCharacterList);
    connect(&m_endpoint_manager, &EndpointManager::characterReceived, this, &MainWindow::receiveCharacter);
    connect(&m_endpoint_manager, &EndpointManager::stashListReceived, this, &MainWindow::receiveStashList);
    connect(&m_endpoint_manager, &EndpointManager::stashReceived, this, &MainWindow::receiveStash);
}

void MainWindow::authenticate()
{
    AuthenticationDialog login;
    connect(&login, &AuthenticationDialog::requested, &m_oauth_manager, &OAuthManager::requestAccess);
    connect(&m_oauth_manager, &OAuthManager::accessGranted, &login, &AuthenticationDialog::setToken);

    if (!m_settings.username().isEmpty()) {
        const OAuthToken token = m_user_data.getStruct<OAuthToken>("oauth_token");
        if (token.isValid()) {
            login.setToken(token);
        };
    };

    int result = login.exec();
    disconnect(&login, &AuthenticationDialog::requested, &m_oauth_manager, nullptr);
    disconnect(&m_oauth_manager, &OAuthManager::accessGranted, &login, nullptr);

    if (result == QDialog::Accepted) {
        m_endpoint_manager.getLeagues();
    };

    activateWindow();
    raise();
}

void MainWindow::receiveLeagueList(std::shared_ptr<std::vector<poe_api::League>> leagues)
{
    m_league_menu.clear();
    for (const auto& league : *leagues) {

        const QString league_name = league.id;

        QString pretty_name = league_name;
        pretty_name.replace("HC ", "Hardcore ");
        pretty_name.replace("R ", "Ruthless ");
        pretty_name.replace("Solo Self-Found", "SSF");

        QAction* action = new QAction(pretty_name, &m_league_menu);
        action->setCheckable(true);
        action->setChecked(false);
        
        connect(action, &QAction::triggered, this,
            [=]() {
                action->setChecked(true);
                if (m_active_league_action) {
                    m_active_league_action->setChecked(false);
                };
                m_active_league_action = action;
                m_settings.setLeague(league_name);
            });

        m_league_menu.addAction(action);
    };
    m_league_menu.setEnabled(true);
    m_refresh_menu.setEnabled(true);
}

void MainWindow::refreshAll()
{
    m_characters_pending = 0;
    m_endpoint_manager.listCharacters();

    m_stashes_pending = 0;
    m_endpoint_manager.listStashes(m_settings.league());
}

void MainWindow::receiveCharacterList(std::shared_ptr<std::vector<poe_api::Character>> characters)
{
    m_character_list = std::move(characters);
    const QString league = m_settings.league();
    for (const auto& character : *m_character_list) {
        if (0 == league.compare(character.league.value_or(""), Qt::CaseInsensitive)) {
            ++m_characters_pending;
            m_endpoint_manager.getCharacter(character.name);
        };
    };
}

void MainWindow::receiveCharacter(std::shared_ptr<poe_api::Character> character)
{
    if (!character) {
        return;
    };
    --m_characters_pending;
    m_league_data.storeCharacter(*character);
    m_tree_model.appendCharacter(*character); 
    m_characters[character->id] = character;
    m_status_label.setText(QString("Waiting to receive %1 stash tabs and %2 characters").arg(
        QString::number(m_stashes_pending),
        QString::number(m_characters_pending)));
}

void MainWindow::receiveStashList(std::shared_ptr<std::vector<poe_api::StashTab>> stashes)
{
    const QString league = m_settings.league();
    m_stash_list = stashes;
    for (const auto& stash : *stashes) {
        ++m_stashes_pending;
        m_endpoint_manager.getStash(league, stash.id);
        if (stash.children) {
            QLOG_ERROR() << "Stash list has children:" << stash.name;
        };
    };
}

void MainWindow::receiveStash(std::shared_ptr<poe_api::StashTab> stash)
{
    if (!stash) {
        return;
    };
    --m_stashes_pending;
    if (stash->children) {
        const QString league = m_settings.league();
        for (const auto& child : stash->children.value()) {
            continue; 
            // TODO
            ++m_stashes_pending;
            m_endpoint_manager.getStash(league, stash->id, child.id);
            if (child.children) {
                QLOG_ERROR() << "Stash has grandchildren:" << stash->name;
            };
        };
    };
    m_league_data.storeStash(*stash);
    m_tree_model.appendStash(*stash);
    m_stashes[stash->id] = stash;
    m_status_label.setText(QString("Waiting to receive %1 stash tabs and %2 characters").arg(
        QString::number(m_stashes_pending),
        QString::number(m_characters_pending)));
}
