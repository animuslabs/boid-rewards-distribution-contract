#pragma once
#include <eosio/eosio.hpp>
#include "../tables/tables.hpp"

namespace gamerewards {
    using namespace eosio;

    class [[eosio::contract("gamerewards")]] game_config {
    protected:
        name contract_account;

    public:
        game_config(name contract_acct) 
            : contract_account(contract_acct) {}

        [[eosio::action]]
        void addgame(
            name game_name,
            string display_name,
            string metadata,
            vector<stat_config> stat_configs
        ) {
            require_auth(contract_account);

            // Validate stat configs
            for (const auto& config : stat_configs) {
                check(config.stat_name.length() > 0, "Stat name cannot be empty");
                check(config.display_name.length() > 0, "Display name cannot be empty");
            }

            // Add game config
            gameconfig_table configs(contract_account, contract_account.value);
            auto game_it = configs.find(game_name.value);
            check(game_it == configs.end(), "Game already exists");

            configs.emplace(contract_account, [&](auto& row) {
                row.game_name = game_name;
                row.display_name = display_name;
                row.metadata = metadata;
                row.stat_configs = stat_configs;
            });

            // Initialize reward config
            rewardconfig_table reward_configs(contract_account, contract_account.value);
            auto reward_it = reward_configs.find(game_name.value);
            if (reward_it == reward_configs.end()) {
                reward_configs.emplace(contract_account, [&](auto& r) {
                    r.game_name = game_name;
                    r.total_reward_pool = asset(0, symbol("BOID", 4));
                    r.last_distributed_cycle = 0;
                    r.reward_percentages = {60, 30, 10}; // Default percentages
                });
            }
        }

        [[eosio::action]]
        void updategame(
            name game_name,
            string display_name,
            string metadata,
            vector<stat_config> stat_configs
        ) {
            require_auth(contract_account);

            // Validate stat configs
            for (const auto& config : stat_configs) {
                check(config.stat_name.length() > 0, "Stat name cannot be empty");
                check(config.display_name.length() > 0, "Display name cannot be empty");
            }

            // Update game config
            gameconfig_table configs(contract_account, contract_account.value);
            auto game_it = configs.find(game_name.value);
            check(game_it != configs.end(), "Game not found");

            configs.modify(game_it, contract_account, [&](auto& row) {
                row.display_name = display_name;
                row.metadata = metadata;
                row.stat_configs = stat_configs;
            });
        }

        [[eosio::action]]
        void removegame(name game_name) {
            require_auth(contract_account);

            // Remove game config
            gameconfig_table configs(contract_account, contract_account.value);
            auto game_it = configs.find(game_name.value);
            check(game_it != configs.end(), "Game not found");
            configs.erase(game_it);

            // Remove reward config
            rewardconfig_table reward_configs(contract_account, contract_account.value);
            auto reward_it = reward_configs.find(game_name.value);
            if (reward_it != reward_configs.end()) {
                reward_configs.erase(reward_it);
            }
        }
    };
}
