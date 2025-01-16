#pragma once
#include <eosio/eosio.hpp>
#include "../tables/tables.hpp"

namespace gamerewards {
    using namespace eosio;

    class [[eosio::contract("gamerewards")]] game_record : public eosio::contract {
    private:
        global_singleton globals;

        void validate_stats(const gameconfig& config, const std::map<name, uint64_t>& stats) {
            for (const auto& stat_pair : stats) {
                const auto& stat_name_to_check = stat_pair.first;
                auto stat_it = std::find_if(config.stat_configs.begin(), config.stat_configs.end(),
                    [stat_name_to_check](const auto& cfg) { return cfg.stat_name == stat_name_to_check; });
                check(stat_it != config.stat_configs.end(), "Stat not configured for game: " + stat_name_to_check.to_string());
            }
        }

        uint32_t determine_cycle(const global_state& global, time_point_sec completion_time) {
            // Calculate how many cycles have passed since start
            uint32_t elapsed_sec = completion_time.sec_since_epoch() - global.cycle_start_time.sec_since_epoch();
            return (elapsed_sec / global.cycle_length_sec) + 1;
        }

        bool is_cycle_rewards_distributed(name game_name, uint32_t cycle) {
            rewardconfig_table rewardconfigs(_self, _self.value);
            auto reward_config = rewardconfigs.find(game_name.value);
            if (reward_config != rewardconfigs.end()) {
                return cycle <= reward_config->last_distributed_cycle;
            }
            return false;
        }

    public:
        game_record(name receiver, name code, datastream<const char*> ds)
            : eosio::contract(receiver, code, ds), globals(_self, _self.value) {}

        [[eosio::action]]
        void recordgame(name game_name, name player, std::map<name, uint64_t> stats, time_point_sec completion_time) {
            require_auth(_self);
    
            // Get game config and validate
            gameconfig_table configs(_self, _self.value);
            auto config = configs.get(game_name.value, "Game not found");
            check(config.active, "Game is not active");

            // Validate stats against config
            validate_stats(config, stats);

            // Get current cycle info
            check(globals.exists(), "Contract not initialized");
            auto global_state = globals.get();

            // Validate completion time
            auto now = time_point_sec(current_time_point());
            check(completion_time <= now, "Game completion time cannot be in the future");
            check(completion_time >= global_state.cycle_start_time, "Game completion time cannot be before contract initialization");

            // Determine which cycle this game belongs to
            uint32_t target_cycle = determine_cycle(global_state, completion_time);

            // Check if rewards have already been distributed for this cycle
            check(!is_cycle_rewards_distributed(game_name, target_cycle), 
                  "Cannot record stats for cycles where rewards have been distributed");

            // Save game record to history
            // Each game played is permanently stored with all its stats
            statshistory_table stats_history(_self, _self.value);
            stats_history.emplace(_self, [&](auto& h) {
                h.id = stats_history.available_primary_key();
                h.game_name = game_name;
                h.boid_id = player;
                h.cycle_number = target_cycle;
                h.stats = stats;
                h.timestamp = completion_time;
            });

            // Emit stats recording event
            action(
                permission_level{_self, "active"_n},
                _self,
                "statsrecord"_n,
                std::make_tuple(game_name, player, stats, target_cycle, completion_time)
            ).send();
        }
    };
}
