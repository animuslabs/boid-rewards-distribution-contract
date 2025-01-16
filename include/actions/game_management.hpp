#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include "../tables/tables.hpp"

namespace gamerewards {
    using namespace eosio;

    class [[eosio::contract("gamerewards")]] game_management : public contract {
    public:
        game_management(name receiver, name code, datastream<const char*> ds)
            : contract(receiver, code, ds) {}

        [[eosio::action]]
        void addgame(name game_name, string display_name, string metadata, vector<stat_config> stat_configs) {
            require_auth(_self);

            gameconfig_table configs(_self, _self.value);
            check(configs.find(game_name.value) == configs.end(), "Game already exists");

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
            check(config != configs.end(), "Game configuration not found");

            configs.modify(config, _self, [&](auto& row) {
                row.metadata = metadata;
            });
        }

        [[eosio::action]]
        void updategame(name game_name, string display_name, string metadata, vector<stat_config> stat_configs) {
            require_auth(_self);

            gameconfig_table configs(_self, _self.value);
            auto game = configs.get(game_name.value, "Game not found");

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
            check(config != configs.end(), "Game configuration not found");
            
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
