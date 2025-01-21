#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include "../tables/tables.hpp"
#include "../tables/config_tables.hpp"
#include <vector>
#include <string>
#include <numeric> // For std::accumulate
#include <algorithm>

#ifndef CONTRACT_NAME
#define CONTRACT_NAME "gamerewards"
#endif

namespace gamerewards {
    using namespace eosio;
    using std::string;
    using std::vector;

    class [[eosio::contract(CONTRACT_NAME)]] reward_management : public contract {
    public:
        reward_management(eosio::name receiver, eosio::name code, datastream<const char*> ds)
            : contract(receiver, code, ds) {}
            

        [[eosio::action]]
        void distribute(
            uint8_t game_id,
            uint32_t cycle_number,
            eosio::name stat_name,
            eosio::asset total_reward,
            eosio::name token_contract,
            std::vector<uint8_t> reward_percentages
        ) {
            require_auth(_self);

            // Access global state
            global_singleton globals(_self, _self.value);
            check(globals.exists(), "Contract not initialized");
            auto global_state = globals.get();

            // Validate inputs
            check(total_reward.amount > 0, "Total reward must be positive");
            check(!reward_percentages.empty(), "Reward percentages cannot be empty");
            check(reward_percentages.size() <= global_state.max_reward_tiers, "Too many reward tiers");
            uint16_t total_percentage = std::accumulate(reward_percentages.begin(), reward_percentages.end(), 0);
            check(total_percentage == 100, "Reward percentages must sum to 100");

            // Validate game existence
            gameconfig_table configs(_self, _self.value);
            auto game_itr = configs.find(game_id);
            check(game_itr != configs.end(), "Game ID not found");
            check(game_itr->active, "Game is not active");

            // Check if rewards have already been distributed for this cycle
            rewardsrecorded_table rewards_records(_self, _self.value);
            auto rewards_idx = rewards_records.get_index<"bygamecycle"_n>();
            uint128_t cycle_key = ((uint128_t)game_id << 64) | cycle_number;
            auto reward_itr = rewards_idx.find(cycle_key);
            check(reward_itr == rewards_idx.end(), "Rewards have already been distributed for this cycle");

            // Validate token configuration
            tokenconfig_table token_configs(_self, _self.value);
            auto token_cfg = token_configs.find(total_reward.symbol.raw());
            check(token_cfg != token_configs.end(), "Token not configured");
            check(token_cfg->enabled, "Token is disabled");
            check(token_cfg->token_contract == token_contract, "Invalid token contract");

            // Fetch player stats from gamerecords
            gamerecords_table game_records(_self, _self.value);
            auto idx = game_records.get_index<"bygamecycle"_n>();

            // Fetch reward distribution configuration
            rewarddistconfig_table distconfigs(_self, _self.value);
            auto dist_itr = distconfigs.find(game_id);
            check(dist_itr != distconfigs.end(), "Reward distribution config not found for game ID");
            const auto& dist_config = *dist_itr;

            // Validate cycle
            uint32_t current_cycle = global_state.get_current_cycle();
            check(cycle_number > 0, "Invalid cycle number: must be greater than zero");
            check(cycle_number < current_cycle, "Cannot distribute rewards for an ongoing or future cycle");

            std::vector<std::pair<eosio::name, uint64_t>> player_scores;

            // Collect scores for all records in the cycle
            for (auto it = idx.lower_bound(cycle_key);
                 it != idx.end() && it->game_id == game_id && it->cycle_number == cycle_number;
                 ++it) {
                // Find the index of the stat_name in stats_names
                auto stat_itr = std::find(it->stats_names.begin(), it->stats_names.end(), stat_name);
                if (stat_itr != it->stats_names.end()) {
                    size_t index = std::distance(it->stats_names.begin(), stat_itr);
                    player_scores.emplace_back(it->player, it->stats_values[index]);
                }
            }

            // Sort players by performance (highest scores first)
            std::sort(player_scores.begin(), player_scores.end(), [&](const auto& a, const auto& b) {
                return a.second > b.second;
            });

            // Select top N players based on global max_reward_tiers
            if (player_scores.size() > global_state.max_reward_tiers) {
                player_scores.resize(global_state.max_reward_tiers);
            }

            // Distribute rewards and record the distribution
            distribute_rewards(player_scores, total_reward, reward_percentages, token_contract, dist_config, game_id, cycle_number, stat_name);

            // Update all records in the cycle as rewards distributed
            for (auto it = idx.lower_bound(cycle_key); 
                it != idx.end() && it->game_id == game_id && it->cycle_number == cycle_number; 
                ++it) {
                game_records.modify(*it, _self, [&](auto& row) {
                    row.rewards_distributed = true;
                });
            }
        }

        void distribute_rewards(
            const std::vector<std::pair<name, uint64_t>>& sorted_players,
            const asset& total_reward,
            const std::vector<uint8_t>& reward_percentages,
            eosio::name token_contract,
            const rewarddistconfig& dist_config,
            uint8_t game_id,
            uint32_t cycle_number,
            eosio::name stat_name
        ) {
            size_t current_tier = 0;
            std::vector<name> rewarded_players;
            std::vector<eosio::asset> player_rewards;
            asset remaining_reward = total_reward;
            asset distributed_total(0, total_reward.symbol);

            for (const auto& [player, stat_value] : sorted_players) {
                if (current_tier >= reward_percentages.size()) break;

                uint8_t percentage = reward_percentages[current_tier];
                asset reward = total_reward * percentage / 100;

                // Adjust reward to token precision
                reward.amount = (reward.amount / total_reward.symbol.precision()) * total_reward.symbol.precision();
                check(reward.amount > 0, "Reward amount too small to distribute");
                
                remaining_reward -= reward;
                distributed_total += reward;

                // Generate memo using the template
                std::string memo = dist_config.memo_template;
                size_t player_pos = memo.find("{{player}}");
                if (player_pos != std::string::npos) {
                    memo.replace(player_pos, 10, player.to_string());
                }

                // Perform transfer
                if (dist_config.use_direct_transfer) {
                    // Direct transfer to the player
                    action(
                        permission_level{_self, "active"_n},
                        token_contract,
                        "transfer"_n,
                        std::make_tuple(_self, player, reward, memo)
                    ).send();
                } else {
                    // Transfer to destination contract
                    action(
                        permission_level{_self, "active"_n},
                        token_contract,
                        "transfer"_n,
                        std::make_tuple(_self, dist_config.destination_contract, reward, memo)
                    ).send();
                }

                rewarded_players.push_back(player);
                player_rewards.push_back(reward);
                current_tier++;
            }

            // Handle any remaining tokens
            if (remaining_reward.amount > 0) {
                // Optionally redistribute or handle the remaining reward
                // Add remaining tokens to the top player or keep in contract
                print("Remaining reward after distribution: ", remaining_reward.to_string(), "\n");
            }

            // Record the distribution in rewardsrecorded table
            rewardsrecorded_table rewards_records(_self, _self.value);
            rewards_records.emplace(_self, [&](auto& record) {
                record.id = rewards_records.available_primary_key();
                record.game_id = game_id;
                record.cycle_number = cycle_number;
                record.stat_name = stat_name;
                record.total_reward = distributed_total;  // Actual amount distributed
                record.rewarded_players = rewarded_players;
                record.player_rewards = player_rewards;
                record.distribution_time = eosio::current_time_point();
            });
        }

    
        [[eosio::action]]
        void setdistconf(uint8_t game_id, eosio::name destination_contract, std::string memo_template, bool use_direct_transfer = false) {
            require_auth(_self);

            // Validate game existence
            gameconfig_table configs(_self, _self.value);
            auto game_itr = configs.find(game_id);
            check(game_itr != configs.end(), "Game ID not found");

            // Validate destination contract
            check(is_account(destination_contract), "Invalid destination contract");

            // Validate memo template
            check(!memo_template.empty(), "Memo template cannot be empty");

            // Add or update reward distribution configuration
            rewarddistconfig_table distconfigs(_self, _self.value);
            auto dist_itr = distconfigs.find(game_id);
            if (dist_itr == distconfigs.end()) {
                distconfigs.emplace(_self, [&](auto& row) {
                    row.game_id = game_id;
                    row.destination_contract = destination_contract;
                    row.memo_template = memo_template;
                    row.use_direct_transfer = use_direct_transfer;
                });
            } else {
                distconfigs.modify(dist_itr, _self, [&](auto& row) {
                    row.destination_contract = destination_contract;
                    row.memo_template = memo_template;
                    row.use_direct_transfer = use_direct_transfer;
                });
            }
        }
    
        [[eosio::action]]
        void settoken(name token_contract, symbol token_symbol, bool enabled) {
            require_auth(_self);

            gamerewards::tokenconfig_table tokens(_self, _self.value);
            auto token_itr = tokens.find(token_symbol.raw());

            if (token_itr == tokens.end()) {
                // Add new token
                tokens.emplace(_self, [&](auto& row) {
                    row.token_contract = token_contract;
                    row.token_symbol = token_symbol;
                    row.enabled = enabled;
                });
            } else {
                // Update existing token
                tokens.modify(token_itr, _self, [&](auto& row) {
                    row.token_contract = token_contract;
                    row.enabled = enabled;
                });
            }
        }

        [[eosio::action]]
        void removetoken(symbol token_symbol) {
            require_auth(_self);

            // Access the token configuration table
            gamerewards::tokenconfig_table tokens(_self, _self.value);
            auto token_itr = tokens.find(token_symbol.raw());
            check(token_itr != tokens.end(), "Token not found");

            tokens.erase(token_itr);
        }

    };
}