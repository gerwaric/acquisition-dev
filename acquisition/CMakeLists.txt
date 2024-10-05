target_include_directories(acquisition PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Acquisition
target_sources(acquisition PRIVATE
    #"acquisition/application.cpp"
    #"acquisition/application.h"
    "acquisition/authentication_dialog.cpp"
    "acquisition/authentication_dialog.h"
    "acquisition/constants.h"
    #"acquisition/datastore.cpp"
    #"acquisition/datastore.h"
    "acquisition/endpoint_manager.cpp"
    "acquisition/endpoint_manager.h"
    #"acquisition/item_model.cpp"
    #"acquisition/item_model.h"
    #"acquisition/items_manager.cpp"
    #"acquisition/items_manager.h"
    "acquisition/main.cpp"
    "acquisition/main_window.cpp"
    "acquisition/main_window.h"
    "acquisition/model_item.cpp"
    "acquisition/model_item.h"
    "acquisition/settings.cpp"
    "acquisition/settings.h"
    "acquisition/tree_model.cpp"
    "acquisition/tree_model.h"
    "acquisition/tree_node.cpp"
    "acquisition/tree_node.h"
)

# Databases
target_sources(acquisition PRIVATE
    #"acquisition/db/data_store.cpp"
    #"acquisition/db/data_store.h"
    "acquisition/databases/league_database.cpp"
    "acquisition/databases/league_database.h"
    "acquisition/databases/user_database.h"
    "acquisition/databases/user_database.cpp"
)

# Utils
target_sources(acquisition PRIVATE
    "acquisition/utils/fatal_error.cpp"
    "acquisition/utils/fatal_error.h"
    "acquisition/utils/json.h"
    "acquisition/utils/utils.cpp"
    "acquisition/utils/utils.h"
)

# OAuth authentication
target_sources(acquisition PRIVATE
    "acquisition/oauth/oauth_manager.cpp"
    "acquisition/oauth/oauth_manager.h"
    "acquisition/oauth/oauth_token.cpp"
    "acquisition/oauth/oauth_token.h"
)

# API rate limiting
target_sources(acquisition PRIVATE
    "acquisition/rate_limit/rate_limited_reply.cpp"
    "acquisition/rate_limit/rate_limited_reply.h"
    "acquisition/rate_limit/rate_limited_request.cpp"
    "acquisition/rate_limit/rate_limited_request.h"
    "acquisition/rate_limit/rate_limiter.cpp"
    "acquisition/rate_limit/rate_limiter.h"
    "acquisition/rate_limit/rate_limit_manager.cpp"
    "acquisition/rate_limit/rate_limit_manager.h"
    "acquisition/rate_limit/rate_limit_policy.cpp"
    "acquisition/rate_limit/rate_limit_policy.h"
    "acquisition/rate_limit/rate_limit_status_dialog.cpp"
    "acquisition/rate_limit/rate_limit_status_dialog.h"
)

# API endpoints
target_sources(acquisition PRIVATE
    "acquisition/api_endpoints/account_characters.cpp"
    "acquisition/api_endpoints/account_characters.h"
    "acquisition/api_endpoints/account_stashes.cpp"
    "acquisition/api_endpoints/account_stashes.h"
)

# API data structures
target_sources(acquisition PRIVATE
    "acquisition/api_types/account.h"
    "acquisition/api_types/character.h"
    "acquisition/api_types/crucible_node.h"
    "acquisition/api_types/display_mode.h"
    "acquisition/api_types/event_ladder_entry.h"
    "acquisition/api_types/frame_type.h"
    "acquisition/api_types/guild.h"
    "acquisition/api_types/item.h"
    "acquisition/api_types/item_filter.h"
    "acquisition/api_types/item_jewel_data.h"
    "acquisition/api_types/item_property.h"
    "acquisition/api_types/item_socket.h"
    "acquisition/api_types/ladder_entry.h"
    "acquisition/api_types/league.h"
    "acquisition/api_types/league_account.h"
    "acquisition/api_types/league_rule.h"
    "acquisition/api_types/legacy_character.h"
    "acquisition/api_types/legacy_stash_tab.h"
    "acquisition/api_types/passive_group.h"
    "acquisition/api_types/passive_node.h"
    "acquisition/api_types/public_stash_change.h"
    "acquisition/api_types/pvp_ladder_team_entry.h"
    "acquisition/api_types/pvp_ladder_team_member.h"
    "acquisition/api_types/pvp_match.h"
    "acquisition/api_types/stash_tab.h"
)
