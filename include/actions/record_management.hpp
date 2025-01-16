#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include "../tables/tables.hpp"
#include "game_record.hpp"

#ifndef CONTRACT_NAME
#define CONTRACT_NAME "gamerewards"
#endif

namespace gamerewards {
    using namespace eosio;

    class [[eosio::contract(CONTRACT_NAME)]] record_management : public contract {
    private:
        void process_single_record(const game_record_data& record, uint32_t cycle) {
            // Validate game exists and stats are configured
            gameconfig_table configs(_self, _self.value);
            auto config = configs.find(record.game_name.value);
            std::string error_msg = "Game configuration for '" + record.game_name.to_string() + "' not found";
            check(config != configs.end(), error_msg.c_str());
            
            error_msg = "Game '" + record.game_name.to_string() + "' is not active";
            check(config->active, error_msg.c_str());

            // Validate all stats exist in config
            for (const auto& stat_pair : record.stats) {
                const auto& stat_name = stat_pair.first;
                bool stat_found = false;
                for (const auto& stat_config : config->stat_configs) {
                    if (stat_config.stat_name == stat_name) {
                        stat_found = true;
                        break;
                    }
                }
                error_msg = "Stat '" + stat_name.to_string() + "' not configured for game '" + record.game_name.to_string() + "'";
                check(stat_found, error_msg.c_str());
            }

            // Get or create player stats
            playerstats_table player_stats(_self, _self.value);
            auto player_game_idx = player_stats.get_index<"byplayergame"_n>();
            uint128_t player_game_key = ((uint128_t)record.player.value << 64) | record.game_name.value;
            auto player_game_it = player_game_idx.find(player_game_key);

            if (player_game_it == player_game_idx.end()) {
                // Create new player stats
                player_stats.emplace(_self, [&](auto& row) {
                    row.id = player_stats.available_primary_key();
                    row.boid_id = record.player;
                    row.game_name = record.game_name;
                    row.stats = record.stats;
                    row.cycle_number = cycle;
                    row.game_completion_time = record.completion_time;
                    row.last_updated = current_time_point();
                    row.rewards_distributed = false;
                });
            } else {
                // Update existing player stats
                player_stats.modify(*player_game_it, _self, [&](auto& row) {
                    // If it's a new cycle, reset cycle stats
                    if (row.cycle_number != cycle) {
                        row.cycle_number = cycle;
                        row.stats = record.stats;
                        row.rewards_distributed = false;
                    } else {
                        // Update cycle stats
                        for (const auto& stat_pair : record.stats) {
                            const auto& stat_name = stat_pair.first;
                            const auto& value = stat_pair.second;
                            row.stats[stat_name] = value;
                        }
                    }
                    row.game_completion_time = record.completion_time;
                    row.last_updated = current_time_point();
                });
            }

            // Record in history
            statshistory_table history(_self, _self.value);
            for (const auto& stat_pair : record.stats) {
                const auto& stat_name = stat_pair.first;
                const auto& value = stat_pair.second;
                history.emplace(_self, [&](auto& row) {
                    row.id = history.available_primary_key();
                    row.boid_id = record.player;
                    row.game_name = record.game_name;
                    row.stats[stat_name] = value;
                    row.cycle_number = cycle;
                    row.timestamp = current_time_point();
                });
            }
        }

    public:
        record_management(name receiver, name code, datastream<const char*> ds)
            : contract(receiver, code, ds) {}

        [[eosio::action]]
        void recordgame(vector<game_record_data> records) {
            require_auth(_self);
            check(!records.empty(), "No game records provided for recording");
            check(records.size() <= 100, "Too many records in a single transaction. Maximum allowed: 100");

            // Get current cycle
            rewardconfig_table reward_configs(_self, _self.value);
            
            for (const auto& record : records) {
                auto config = reward_configs.find(record.game_name.value);
                check(config != reward_configs.end(), "Game reward config not found");
                uint32_t current_cycle = config->last_distributed_cycle + 1;
                
                process_single_record(record, current_cycle);
            }
        }

        [[eosio::action]]
        void getrecord(uint64_t id) {
            // This action just reads data, no auth required
            playerstats_table player_stats(_self, _self.value);
            auto stats = player_stats.get(id, "Record not found");
            
            // This print statement is for testing/viewing purposes
            print("Record found - Player: ", stats.boid_id, 
                  ", Game: ", stats.game_name,
                  ", Cycle: ", stats.cycle_number);
        }

        [[eosio::action]]
        void clearrecord(uint64_t id) {
            require_auth(_self);
            
            playerstats_table player_stats(_self, _self.value);
            auto stats = player_stats.find(id);
            check(stats != player_stats.end(), "Record not found");
            check(!stats->rewards_distributed, "Cannot clear record after rewards distributed");
            
            player_stats.erase(stats);
        }
    };
}
