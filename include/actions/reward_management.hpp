#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include "../tables/tables.hpp"
#include <algorithm>
#include <numeric>

namespace gamerewards {
    using namespace eosio;

    class [[eosio::contract("gamerewards")]] reward_management : public contract {
    protected:
        name contract_account;
        static constexpr uint8_t MAX_REWARD_TIERS = 10;
        static constexpr uint8_t MIN_REWARD_PERCENTAGE = 1;
        static constexpr uint8_t MAX_TOTAL_PERCENTAGE = 100;

        gameconfig_table configs;
        playerstats_table player_stats;
        reward_distribution_table dist_configs;
        rewardconfig_table reward_configs;
        statshistory_table stats_history;
        cycledistribution_table cycle_dist;

    public:
        reward_management(name receiver, name code, datastream<const char*> ds)
            : contract(receiver, code, ds), contract_account(receiver),
              configs(contract_account, contract_account.value),
              player_stats(contract_account, contract_account.value),
              dist_configs(contract_account, contract_account.value),
              reward_configs(contract_account, contract_account.value),
              stats_history(contract_account, contract_account.value),
              cycle_dist(contract_account, contract_account.value) {}

        [[eosio::action]]
        void setdistconf(
            name game_name,
            name destination_contract,
            std::string memo_template
        ) {
            require_auth(contract_account);

            // Validate memo template
            check(memo_template.length() <= 256, "Memo template too long");
            check(memo_template.find("{cycle}") != std::string::npos, "Memo template must contain {cycle} placeholder");
            check(memo_template.find("{player}") != std::string::npos, "Memo template must contain {player} placeholder");

            // Get or create reward distribution config
            auto config_it = dist_configs.find(game_name.value);

            if (config_it == dist_configs.end()) {
                dist_configs.emplace(contract_account, [&](auto& row) {
                    row.game_name = game_name;
                    row.destination_contract = destination_contract;
                    row.memo_template = memo_template;
                });
            } else {
                dist_configs.modify(config_it, contract_account, [&](auto& row) {
                    row.destination_contract = destination_contract;
                    row.memo_template = memo_template;
                });
            }
        }

        [[eosio::action]]
        void distribute(
            name game_name,
            uint32_t cycle_number,
            name stat_name,
            asset total_reward,
            name token_contract,
            std::vector<uint8_t> reward_percentages,
            bool normalize_percentages = false
        ) {
            require_auth(contract_account);

            // Validate input parameters
            check(reward_percentages.size() <= MAX_REWARD_TIERS, "Too many reward tiers");
            check(!reward_percentages.empty(), "Must specify at least one reward tier");
            check(total_reward.amount > 0, "Total reward must be positive");

            // Validate percentages
            uint16_t total_percentage = 0;
            for (uint8_t percentage : reward_percentages) {
                check(percentage > 0, "Reward percentage must be greater than 0");
                total_percentage += percentage;
            }
            check(total_percentage == 100, "Reward percentages must sum to 100");

            // Get game config
            auto game_config = configs.find(game_name.value);
            check(game_config != configs.end(), "Game not found");
            check(game_config->active, "Game is not active");

            // Verify stat exists and is configured for rewards
            auto stat_it = std::find_if(game_config->stat_configs.begin(), game_config->stat_configs.end(),
                [&](const auto& cfg) { return cfg.stat_name == stat_name; });
            check(stat_it != game_config->stat_configs.end(), "Stat not configured for game");
            bool is_high_better = stat_it->is_high_better;

            // Get all records from history for this cycle
            statshistory_table stats_history(contract_account, contract_account.value);
            auto by_cycle = stats_history.get_index<"bycycle"_n>();
            
            // Create composite key for cycle lookup
            uint128_t cycle_key = ((uint128_t)cycle_number << 64);
            uint128_t next_cycle_key = ((uint128_t)(cycle_number + 1) << 64);
            
            // Structure to track player's best performance
            struct PlayerStats {
                uint64_t best_value = 0;
                uint64_t games_played = 0;
                time_point_sec last_played;
            };
            
            // Map to store best stat value and metadata for each player
            std::map<name, PlayerStats> player_stats;
            
            // Analyze all games played in this cycle
            auto it = by_cycle.lower_bound(cycle_key);
            auto end_it = by_cycle.lower_bound(next_cycle_key);
            
            while (it != end_it) {
                // Only process records for the target game
                if (it->game_name == game_name && it->stats.contains(stat_name)) {
                    uint64_t stat_value = it->stats.at(stat_name);
                    auto& stats = player_stats[it->boid_id];
                    
                    // Update games played count and last played time
                    stats.games_played++;
                    if (stats.last_played < it->timestamp) {
                        stats.last_played = it->timestamp;
                    }
                    
                    // First game or better performance
                    if (stats.games_played == 1) {
                        stats.best_value = stat_value;
                    } else if (is_high_better && stat_value > stats.best_value) {
                        stats.best_value = stat_value;
                    } else if (!is_high_better && stat_value < stats.best_value) {
                        stats.best_value = stat_value;
                    }
                }
                it++;
            }

            // Convert map to vector for sorting
            std::vector<std::pair<name, uint64_t>> rankings;
            for (const auto& [player, stats] : player_stats) {
                rankings.push_back({player, stats.best_value});
            }

            check(!rankings.empty(), "No eligible players found for rewards");

            // Sort players by their best performance
            std::sort(rankings.begin(), rankings.end(),
                [&](const auto& a, const auto& b) {
                    return is_high_better ? a.second > b.second : a.second < b.second;
                });

            // Group players by value to handle ties
            std::vector<std::vector<std::pair<name, uint64_t>>> reward_tiers;
            if (!rankings.empty()) {
                std::vector<std::pair<name, uint64_t>> current_tier = {rankings[0]};
                uint64_t current_value = rankings[0].second;

                for (size_t i = 1; i < rankings.size(); ++i) {
                    if (rankings[i].second == current_value) {
                        current_tier.push_back(rankings[i]);
                    } else {
                        reward_tiers.push_back(current_tier);
                        current_tier = {rankings[i]};
                        current_value = rankings[i].second;
                    }
                }
                reward_tiers.push_back(current_tier);
            }

            // Record this distribution
            cycle_dist.emplace(contract_account, [&](auto& row) {
                row.id = cycle_dist.available_primary_key();
                row.game_name = game_name;
                row.cycle_number = cycle_number;
                row.stat_name = stat_name;
                row.total_reward = total_reward;
                row.distribution_time = time_point_sec(current_time_point());
            });

            // Update reward config
            auto config_it = reward_configs.find(game_name.value);
            if (config_it == reward_configs.end()) {
                reward_configs.emplace(contract_account, [&](auto& row) {
                    row.game_name = game_name;
                    row.total_reward_pool = total_reward;
                    row.start_cycle = cycle_number;
                    row.end_cycle = cycle_number;
                    row.last_distributed_cycle = cycle_number;
                    row.reward_percentages = reward_percentages;
                });
            } else {
                reward_configs.modify(config_it, contract_account, [&](auto& row) {
                    row.total_reward_pool = total_reward;
                    row.end_cycle = cycle_number;
                    row.last_distributed_cycle = cycle_number;
                    row.reward_percentages = reward_percentages;
                });
            }

            // Get distribution config for memo template
            auto dist_config = dist_configs.get(game_name.value, "Distribution config not found");
            std::string base_memo = dist_config.memo_template;
            
            // Replace placeholders in memo template
            size_t cycle_pos = base_memo.find("{cycle}");
            if (cycle_pos != std::string::npos) {
                base_memo.replace(cycle_pos, 7, std::to_string(cycle_number));
            }

            // Distribute rewards to each tier
            size_t current_tier = 0;
            for (const auto& tier : reward_tiers) {
                if (current_tier >= reward_percentages.size()) break;

                uint8_t percentage = reward_percentages[current_tier++];
                if (percentage == 0) continue;

                // Calculate reward for this tier
                asset tier_reward = total_reward;
                tier_reward.amount = (tier_reward.amount * percentage) / 100;

                // Split reward among tied players
                asset player_reward = asset{tier_reward.amount / tier.size(), tier_reward.symbol};
                check(player_reward.amount > 0, "Reward amount too small to split among tied players");

                // Send reward to each player in the tier
                for (const auto& [player_id, stat_value] : tier) {
                    // Create memo from template, replacing {player} placeholder
                    std::string memo = base_memo;
                    size_t player_pos = memo.find("{player}");
                    if (player_pos != std::string::npos) {
                        memo.replace(player_pos, 8, player_id.to_string());
                    }

                    action(
                        permission_level{contract_account, "active"_n},
                        dist_config.destination_contract,
                        "transfer"_n,
                        std::make_tuple(contract_account, player_id, player_reward, memo)
                    ).send();
                }
            }

            // Emit distribution event
            action(
                permission_level{contract_account, "active"_n},
                contract_account,
                "rewarddist"_n,
                std::make_tuple(game_name, cycle_number, stat_name, total_reward, rankings)
            ).send();
        }

        [[eosio::action]]
        void findmissing(name game_name, uint32_t start_cycle, uint32_t end_cycle) {
            require_auth(contract_account);

            check(start_cycle <= end_cycle, "Start cycle must be less than or equal to end cycle");

            // Get all distributions for this game between start and end cycles
            cycledistribution_table cycle_dist(contract_account, contract_account.value);
            auto by_game_cycle = cycle_dist.get_index<"bygamecycle"_n>();
            
            uint128_t start_key = ((uint128_t)game_name.value << 64) | start_cycle;
            uint128_t end_key = ((uint128_t)game_name.value << 64) | end_cycle;

            // Set to store cycles that have distributions
            std::set<uint32_t> distributed_cycles;
            
            auto it = by_game_cycle.lower_bound(start_key);
            auto end_it = by_game_cycle.upper_bound(end_key);
            
            while (it != end_it) {
                distributed_cycles.insert(it->cycle_number);
                it++;
            }

            // Find missing cycles
            vector<uint32_t> missing_cycles;
            for (uint32_t cycle = start_cycle; cycle <= end_cycle; cycle++) {
                if (distributed_cycles.find(cycle) == distributed_cycles.end()) {
                    missing_cycles.push_back(cycle);
                }
            }

            // Emit event with missing cycles
            action(
                permission_level{contract_account, "active"_n},
                contract_account,
                "missingcycle"_n,
                std::make_tuple(game_name, missing_cycles)
            ).send();
        }

        [[eosio::action]]
        void trimhistory(name game_name, time_point_sec start_time, time_point_sec end_time, uint32_t max_deletions = 1000) {
            require_auth(contract_account);

            // Validate time range
            check(start_time < end_time, "Start time must be before end time");
            check(end_time < current_time_point(), "End time cannot be in the future");

            // Get global state to determine cycles
            global_singleton globals(contract_account, contract_account.value);
            check(globals.exists(), "Contract not initialized");
            auto global_state = globals.get();

            // Calculate cycle numbers for the time range
            uint32_t start_cycle = (start_time.sec_since_epoch() - global_state.cycle_start_time.sec_since_epoch()) / global_state.cycle_length_sec + 1;
            uint32_t end_cycle = (end_time.sec_since_epoch() - global_state.cycle_start_time.sec_since_epoch()) / global_state.cycle_length_sec + 1;

            // Get all distributions for these cycles to ensure they're distributed
            cycledistribution_table cycle_dist(contract_account, contract_account.value);
            auto by_game_cycle = cycle_dist.get_index<"bygamecycle"_n>();
            
            uint128_t start_key = ((uint128_t)game_name.value << 64) | start_cycle;
            uint128_t end_key = ((uint128_t)game_name.value << 64) | end_cycle;

            // Set to store cycles that have distributions
            std::set<uint32_t> distributed_cycles;
            
            auto dist_it = by_game_cycle.lower_bound(start_key);
            auto dist_end = by_game_cycle.upper_bound(end_key);
            
            while (dist_it != dist_end) {
                distributed_cycles.insert(dist_it->cycle_number);
                dist_it++;
            }

            // Get history records for the time range
            statshistory_table stats_history(contract_account, contract_account.value);
            
            uint32_t deleted_count = 0;
            auto history_it = stats_history.begin();
            
            while (history_it != stats_history.end() && deleted_count < max_deletions) {
                if (history_it->game_name == game_name && 
                    history_it->timestamp >= start_time && 
                    history_it->timestamp <= end_time) {
                    
                    // Calculate which cycle this record belongs to
                    uint32_t record_cycle = (history_it->timestamp.sec_since_epoch() - global_state.cycle_start_time.sec_since_epoch()) / global_state.cycle_length_sec + 1;
                    
                    // Only delete if the cycle has been distributed
                    if (distributed_cycles.find(record_cycle) != distributed_cycles.end()) {
                        history_it = stats_history.erase(history_it);
                        deleted_count++;
                    } else {
                        history_it++;
                    }
                } else {
                    history_it++;
                }
            }

            // Emit event with deletion results
            action(
                permission_level{contract_account, "active"_n},
                contract_account,
                "historytrim"_n,
                std::make_tuple(game_name, start_time, end_time, deleted_count)
            ).send();
        }

    private:
        void send_reward(
            name player,
            asset reward,
            name token_contract,
            const reward_distribution_config& dist_config,
            uint32_t cycle_number
        ) {
            // Format memo
            std::string memo = dist_config.memo_template;
            size_t cycle_pos = memo.find("{cycle}");
            if (cycle_pos != std::string::npos) {
                memo.replace(cycle_pos, 7, std::to_string(cycle_number));
            }
            size_t player_pos = memo.find("{player}");
            if (player_pos != std::string::npos) {
                memo.replace(player_pos, 8, player.to_string());
            }

            // Send reward
            action(
                permission_level{contract_account, "active"_n},
                token_contract,
                "transfer"_n,
                std::make_tuple(
                    contract_account,
                    dist_config.destination_contract,
                    reward,
                    memo
                )
            ).send();
        }

        void mark_rewards_distributed(name player, name game_name, uint32_t cycle_number) {
            playerstats_table player_stats(contract_account, contract_account.value);
            auto by_player_game = player_stats.get_index<"byplayergame"_n>();
            uint128_t composite_key = ((uint128_t)player.value << 64) | game_name.value;

            auto player_it = by_player_game.find(composite_key);
            if (player_it != by_player_game.end() && player_it->cycle_number == cycle_number) {
                by_player_game.modify(player_it, contract_account, [&](auto& row) {
                    row.rewards_distributed = true;
                });
            }
        }
    };
}
