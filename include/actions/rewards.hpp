#pragma once
#include <eosio/eosio.hpp>
#include "../tables/tables.hpp"

namespace gamerewards {
    using namespace eosio;

    class [[eosio::contract("gamerewards")]] rewards {
    protected:
        name contract_account;

    public:
        rewards(name contract_acct) 
            : contract_account(contract_acct) {}

        [[eosio::action]]
        void distribute(
            name game_name,
            uint32_t cycle_number,
            name stat_name
        ) {
            require_auth(contract_account);

            // Get game config
            gamerewards::gameconfig_table configs(contract_account, contract_account.value);
            auto game_config = configs.get(game_name.value, "Game configuration not found");

            // Get reward config
            rewardconfig_table reward_configs(contract_account, contract_account.value);
            auto config = reward_configs.find(game_name.value);
            check(config != reward_configs.end(), "Reward config not found");

            // Check if rewards were already distributed for this cycle
            check(config->last_distributed_cycle < cycle_number, "Rewards already distributed for this cycle");

            // Get top players for this game and cycle
            auto top_players = get_top_players(game_name, cycle_number, stat_name);
            check(!top_players.empty(), "No players found for the specified cycle");

            // Calculate rewards for each player
            for (size_t i = 0; i < std::min(top_players.size(), config->reward_percentages.size()); ++i) {
                const auto& [player, value] = top_players[i];
                uint8_t percentage = config->reward_percentages[i];
                
                asset reward = config->total_reward_pool;
                reward.amount = (reward.amount * percentage) / 100;

                // Send reward to player
                action(
                    permission_level{contract_account, contract_account},
                    config->token_contract,
                    "transfer"_n,
                    std::make_tuple(
                        contract_account,
                        player,
                        reward,
                        std::string("Reward for cycle ") + std::to_string(cycle_number)
                    )
                ).send();
            }

            // Update last distributed cycle
            reward_configs.modify(config, contract_account, [&](auto& r) {
                r.last_distributed_cycle = cycle_number;
            });
        }

    private:
        std::vector<std::pair<name, uint64_t>> get_top_players(
            name game_name,
            uint32_t cycle_number,
            name stat_name
        ) {
            std::vector<std::pair<name, uint64_t>> top_players;

            // Get player stats for this game and cycle
            playerstats_table player_stats(contract_account, contract_account.value);
            auto game_cycle_idx = player_stats.get_index<"bygamecycle"_n>();
            uint128_t composite_key = ((uint128_t)game_name.value << 64) | cycle_number;

            auto cycle_start = game_cycle_idx.lower_bound(composite_key);
            auto cycle_end = game_cycle_idx.upper_bound(composite_key);

            // Collect all players' stats
            for (auto it = cycle_start; it != cycle_end; ++it) {
                if (!it->rewards_distributed && it->cycle_stats.contains(stat_name)) {
                    top_players.push_back({it->boid_id, it->cycle_stats.at(stat_name)});
                }
            }

            // Sort by stat value (descending)
            std::sort(top_players.begin(), top_players.end(),
                [](const auto& a, const auto& b) { return a.second > b.second; });

            return top_players;
        }
    };
}
