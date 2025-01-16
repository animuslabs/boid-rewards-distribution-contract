#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include "../tables/tables.hpp"

#ifndef CONTRACT_NAME
#define CONTRACT_NAME "gamerewards"
#endif

namespace gamerewards {
    using namespace eosio;

    class [[eosio::contract(CONTRACT_NAME)]] game_management : public contract {
    public:
        game_management(name receiver, name code, datastream<const char*> ds)
            : contract(receiver, code, ds) {}

        [[eosio::action]]
        void addgame(name game_name, string display_name, string metadata, vector<stat_config> stat_configs) {
            require_auth(_self);

            gameconfig_table configs(_self, _self.value);
            std::string error_msg = "Game '" + game_name.to_string() + "' already exists";
            check(configs.find(game_name.value) == configs.end(), error_msg.c_str());

            configs.emplace(_self, [&](auto& row) {
                row.game_name = game_name;
                row.display_name = display_name;
                row.metadata = metadata;
                row.stat_configs = stat_configs;
            });
        }

        [[eosio::action]]
        void configgame(
            name game_name,
            std::string metadata
        ) {
            require_auth(_self);

            gameconfig_table configs(_self, _self.value);
            auto config = configs.find(game_name.value);
            std::string error_msg = "Game configuration for '" + game_name.to_string() + "' not found";
            check(config != configs.end(), error_msg.c_str());

            configs.modify(config, _self, [&](auto& row) {
                row.metadata = metadata;
            });
        }

        [[eosio::action]]
        void updategame(name game_name, string display_name, string metadata, vector<stat_config> stat_configs) {
            require_auth(_self);

            gameconfig_table configs(_self, _self.value);
            std::string error_msg = "Game '" + game_name.to_string() + "' not found";
            auto game = configs.get(game_name.value, error_msg.c_str());

            configs.modify(game, _self, [&](auto& row) {
                row.display_name = display_name;
                row.metadata = metadata;
                row.stat_configs = stat_configs;
            });
        }

        [[eosio::action]]
        void removegame(name game_name) {
            require_auth(_self);
            
            gameconfig_table configs(_self, _self.value);
            auto config = configs.find(game_name.value);
            std::string error_msg = "Game configuration for '" + game_name.to_string() + "' not found";
            check(config != configs.end(), error_msg.c_str());
            
            configs.erase(config);
        }

        [[eosio::action]]
        void getconfig(name game_name) {
            gameconfig_table configs(_self, _self.value);
            auto config = configs.find(game_name.value);
            check(config != configs.end(), "Game configuration not found");
        }
    };
}
