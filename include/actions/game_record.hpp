#pragma once
#include <eosio/eosio.hpp>
#include "../tables/tables.hpp"

#ifndef CONTRACT_NAME
#define CONTRACT_NAME "gamerewards"
#endif

namespace gamerewards {

    struct game_record_data {
        name game_name;
        name player;
        std::map<name, uint64_t> stats;
        time_point_sec completion_time;

        EOSLIB_SERIALIZE(game_record_data, (game_name)(player)(stats)(completion_time))
    };

    class [[eosio::contract(CONTRACT_NAME)]] game_record : public eosio::contract {
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

        void process_game_record(const game_record_data& record) {
            // Get game config and validate
            gameconfig_table configs(_self, _self.value);
            auto config = configs.get(record.game_name.value, "Game not found");
            check(config.active, "Game is not active");

            // Validate stats against config
            validate_stats(config, record.stats);

            // Get current cycle info
            check(globals.exists(), "Contract not initialized");
            auto global_state = globals.get();

            // Validate completion time
            auto now = time_point_sec(current_time_point());
            check(record.completion_time <= now, "Game completion time cannot be in the future");
            check(record.completion_time >= global_state.cycle_start_time, "Game completion time cannot be before contract initialization");

            // Determine which cycle this game belongs to
            uint32_t target_cycle = determine_cycle(global_state, record.completion_time);

            // Check if rewards have already been distributed for this cycle
            check(!is_cycle_rewards_distributed(record.game_name, target_cycle), 
                  "Cannot record game for cycle " + std::to_string(target_cycle) + " - rewards already distributed");

            // Record the stats
            playerstats_table playerstats(_self, _self.value);
            playerstats.emplace(_self, [&](auto& row) {
                row.id = playerstats.available_primary_key();
                row.boid_id = record.player;
                row.game_name = record.game_name;
                row.stats = record.stats;
                row.cycle_number = target_cycle;
                row.game_completion_time = record.completion_time;
                row.last_updated = time_point_sec(current_time_point());
            });
        }

    public:
        game_record(name receiver, name code, datastream<const char*> ds)
            : eosio::contract(receiver, code, ds), globals(_self, _self.value) {}

        [[eosio::action]]
        void recordgame(vector<game_record_data> records) {
            require_auth(_self);
            
            check(!records.empty(), "No records provided");
            check(records.size() <= 100, "Too many records in a single transaction");

            for (const auto& record : records) {
                process_game_record(record);
            }
        }
    };
}
