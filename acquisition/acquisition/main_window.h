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

#include <acquisition/api_types/character.h>
#include <acquisition/api_types/league.h>
#include <acquisition/api_types/stash_tab.h>
#include <acquisition/db/global_data_store.h>
#include <acquisition/db/league_data_store.h>
#include <acquisition/endpoint_manager.h>
#include <acquisition/main_window.h>
#include <acquisition/oauth/oauth_manager.h>
#include <acquisition/oauth/oauth_settings.h>
#include <acquisition/rate_limit/rate_limiter.h>
#include <acquisition/settings.h>
#include <acquisition/tree_model.h>

#include <QMainWindow>

#include <QAction>
#include <QComboBox>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <unordered_map>
#include <vector>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow();


 signals:
    void leagueChanged(const QString& league);
    void showRateLimitStatus();

public slots:
    void receiveLeagueList(std::shared_ptr<std::vector<poe_api::League>> leagues);
    void receiveCharacterList(std::shared_ptr<std::vector<poe_api::Character>> characters);
    void receiveCharacter(std::shared_ptr<poe_api::Character> character);
    void receiveStashList(std::shared_ptr<std::vector<poe_api::StashTab>> stashes);
    void receiveStash(std::shared_ptr<poe_api::StashTab> stash);

    //void refreshCharacterList();
    //void refreshCharacter();
    //void refreshStashList();
    //void refreshStash();
    void refreshAll();

private:

    void setupUserInterface();

    void authenticate();

    bool refreshComplete();
    void finishRefresh();

    static const OAuthSettings s_oauth_settings;

    QNetworkAccessManager m_network_manager;

    Settings m_settings;
    LeagueDataStore m_league_data;
    OAuthManager m_oauth_manager;
    RateLimiter m_rate_limiter;
    EndpointManager m_endpoint_manager;

    QString m_current_league;
    QAction* m_current_league_action{ nullptr };

    QStringList m_league_list;
    std::shared_ptr<std::vector<poe_api::Character>> m_character_list;
    std::shared_ptr<std::vector<poe_api::StashTab>> m_stash_list;

    std::unordered_map<QString, std::shared_ptr<poe_api::Character>> m_characters;
    std::unordered_map<QString, std::shared_ptr<poe_api::StashTab>> m_stashes;

    QLabel m_status_label;
    QMenu m_league_menu;
    QMenu m_refresh_menu;
    QVBoxLayout m_central_layout;
    QTreeView m_tree_view;

    TreeModel m_tree_model;

    int characters_pending{ 0 };
    int stashes_pending{ 0 };

};
