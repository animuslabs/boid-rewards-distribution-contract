#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include "../tables/tables.hpp"
#include <vector>
#include <string>

#ifndef CONTRACT_NAME
#define CONTRACT_NAME "gamerewards"
#endif

namespace gamerewards {
    using namespace eosio;
    using std::string;
    using std::vector;

    class [[eosio::contract(CONTRACT_NAME)]] game_management : public contract {
    public:
        game_management(name receiver, name code, datastream<const char*> ds)
            : contract(receiver, code, ds) {}

        /**
         * Validate that stat names are unique.
         * @param stats_names List of stat names to validate.
         */
        void validate_stats(const vector<name>& stats_names) {
            for (size_t i = 0; i < stats_names.size(); ++i) {
                for (size_t j = i + 1; j < stats_names.size(); ++j) {
                    check(stats_names[i] != stats_names[j],
                        "Duplicate stat name found: " + stats_names[i].to_string());
                }
            }
        }

        [[eosio::action]]
        void setgame(
            uint8_t game_id,
            std::string display_name,
            std::string metadata,
            vector<eosio::name> stats_names
        ) {
            require_auth(_self);

            // Validate stat configs
            validate_stats(stats_names);

            gameconfig_table configs(_self, _self.value);
            auto game_itr = configs.find(game_id);

            if (game_itr == configs.end()) {
                // Add a new game
                configs.emplace(_self, [&](auto& row) {
                    row.game_id = game_id;
                    row.display_name = display_name;
                    row.metadata = metadata;
                    row.stats_names = stats_names;
                    row.active = true;
                });
            } else {
                // Update the existing game
                configs.modify(game_itr, _self, [&](auto& row) {
                    row.display_name = display_name;
                    row.metadata = metadata;
                    row.stats_names = stats_names;
                });
            }
        }

        [[eosio::action]]
        void removegame(uint8_t game_id) {
            require_auth(_self);

            gameconfig_table configs(_self, _self.value);
            auto config_itr = configs.find(game_id);
            check(config_itr != configs.end(), "Game ID not found");

            configs.erase(config_itr);
        }
    };
}
