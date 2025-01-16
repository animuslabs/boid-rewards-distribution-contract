#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include "../tables/tables.hpp"

namespace gamerewards {
    using namespace eosio;

    class [[eosio::contract("gamerewards")]] record_management : public contract {
    public:
        record_management(name receiver, name code, datastream<const char*> ds)
            : contract(receiver, code, ds) {}

        [[eosio::action]]
        void recordgame(
            name game_name,
            name player,
            std::map<name, uint64_t> stats,
            uint32_t cycle
        ) {
            require_auth(_self);

            // Validate game exists and stats are configured
            gameconfig_table configs(_self, _self.value);
            auto config = configs.find(game_name.value);
            check(config != configs.end(), "Game configuration not found");
            check(config->active, "Game is not active");

            // Validate all stats exist in config
            for (const auto& stat_pair : stats) {
                const auto& stat_name = stat_pair.first;
                bool stat_found = false;
                for (const auto& stat_config : config->stat_configs) {
                    if (stat_config.stat_name == stat_name) {
                        stat_found = true;
                        break;
                    }
                }
                check(stat_found, "Stat " + stat_name.to_string() + " not configured for game");
            }

            // Get or create player stats
            playerstats_table player_stats(_self, _self.value);
            auto player_game_idx = player_stats.get_index<"byplayergame"_n>();
            uint128_t player_game_key = ((uint128_t)player.value << 64) | game_name.value;
            auto player_game_it = player_game_idx.find(player_game_key);

            if (player_game_it == player_game_idx.end()) {
                // Create new player stats
                player_stats.emplace(_self, [&](auto& row) {
                    row.id = player_stats.available_primary_key();
                    row.boid_id = player;
                    row.game_name = game_name;
                    row.stats = stats;
                    row.cycle_number = cycle;
                    row.last_updated = current_time_point();
                    row.rewards_distributed = false;
                });
            } else {
                // Update existing player stats
                player_stats.modify(*player_game_it, _self, [&](auto& row) {
                    // If it's a new cycle, reset cycle stats
                    if (row.cycle_number != cycle) {
                        row.cycle_number = cycle;
                        row.stats = stats;
                        row.rewards_distributed = false;
                    } else {
                        // Update cycle stats
                        for (const auto& stat_pair : stats) {
                            const auto& stat_name = stat_pair.first;
                            const auto& value = stat_pair.second;
                            row.stats[stat_name] = value;
                        }
                    }
                    row.last_updated = current_time_point();
                });
            }

            // Record in history
            statshistory_table history(_self, _self.value);
            for (const auto& stat_pair : stats) {
                const auto& stat_name = stat_pair.first;
                const auto& value = stat_pair.second;
                history.emplace(_self, [&](auto& row) {
                    row.id = history.available_primary_key();
                    row.boid_id = player;
                    row.game_name = game_name;
                    row.stats[stat_name] = value;
                    row.cycle_number = cycle;
                    row.timestamp = current_time_point();
                });
            }

            // Emit stats recording event
            action(
                permission_level{_self, "active"_n},
                _self,
                "statsrecord"_n,
                std::make_tuple(game_name, player, stats)
            ).send();
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

            // Emit stats clear event
            action(
                permission_level{_self, "active"_n},
                _self,
                "statsclear"_n,
                std::make_tuple(stats->boid_id)
            ).send();
        }

        [[eosio::action]]
        void consolidate(name game_name, uint32_t cycle) {
            require_auth(_self);

            playerstats_table player_stats(_self, _self.value);
            auto game_cycle_idx = player_stats.get_index<"bygamecycle"_n>();
            uint128_t composite_key = ((uint128_t)game_name.value << 64) | cycle;
            
            auto start = game_cycle_idx.lower_bound(composite_key);
            auto end = game_cycle_idx.upper_bound(composite_key);

            // Process each player's stats for this game and cycle
            for (auto it = start; it != end; ++it) {
                // Already consolidated stats are marked with rewards_distributed
                if (!it->rewards_distributed) {
                    player_stats.modify(*it, _self, [&](auto& row) {
                        row.rewards_distributed = true;
                    });
                }
            }

            // Emit consolidation event
            action(
                permission_level{_self, "active"_n},
                _self,
                "consolidate"_n,
                std::make_tuple(game_name, cycle)
            ).send();
        }

        [[eosio::action]]
        void submitstats(
            name game_name,
            name player,
            std::map<name, uint64_t> stats
        ) {
            require_auth(player);

            // Get current cycle from reward config
            rewardconfig_table reward_configs(_self, _self.value);
            auto config = reward_configs.find(game_name.value);
            check(config != reward_configs.end(), "Game reward config not found");
            uint32_t current_cycle = config->last_distributed_cycle + 1;

            // Record game with current cycle
            recordgame(game_name, player, stats, current_cycle);
        }
    };
}
