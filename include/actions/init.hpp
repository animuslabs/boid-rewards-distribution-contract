#pragma once
#include <eosio/eosio.hpp>
#include "../tables/tables.hpp"

namespace gamerewards {
    using namespace eosio;

    class [[eosio::contract("gamerewards")]] init : public contract {
    public:
        init(name receiver, name code, datastream<const char*> ds) 
            : contract(receiver, code, ds) {}

        [[eosio::action]]
        void initcontract(time_point_sec start_time, uint32_t cycle_length_sec, uint32_t max_cycle_length_sec) {
            require_auth(_self);

            // Validate inputs
            check(cycle_length_sec > 0, "Cycle length must be greater than 0");
            check(max_cycle_length_sec > 0, "Max cycle length must be greater than 0");
            check(max_cycle_length_sec >= cycle_length_sec, "Max cycle length cannot be less than cycle length");
            check(start_time.sec_since_epoch() > 0, "Invalid start time");

            auto now = time_point_sec(current_time_point());
            
            // Allow start time to be at most 60 seconds in the past to account for block time variations
            check(start_time.sec_since_epoch() >= now.sec_since_epoch() - 60, "Start time cannot be in the past");
            check(start_time.sec_since_epoch() <= now.sec_since_epoch() + 60, "Start time cannot be in the future");

            // Clear all existing tables
            global_singleton global(_self, _self.value);
            if (global.exists()) {
                global.remove();
            }

            // Clear game configs
            gameconfig_table gameconfigs(_self, _self.value);
            auto game_itr = gameconfigs.begin();
            while (game_itr != gameconfigs.end()) {
                game_itr = gameconfigs.erase(game_itr);
            }

            // Clear player stats
            playerstats_table playerstats(_self, _self.value);
            auto stats_itr = playerstats.begin();
            while (stats_itr != playerstats.end()) {
                stats_itr = playerstats.erase(stats_itr);
            }

            // Clear stats history
            statshistory_table statshistory(_self, _self.value);
            auto history_itr = statshistory.begin();
            while (history_itr != statshistory.end()) {
                history_itr = statshistory.erase(history_itr);
            }

            // Clear reward configs
            rewardconfig_table rewardconfigs(_self, _self.value);
            auto reward_itr = rewardconfigs.begin();
            while (reward_itr != rewardconfigs.end()) {
                reward_itr = rewardconfigs.erase(reward_itr);
            }

            // Clear reward distribution configs
            reward_distribution_table distconfigs(_self, _self.value);
            auto dist_itr = distconfigs.begin();
            while (dist_itr != distconfigs.end()) {
                dist_itr = distconfigs.erase(dist_itr);
            }

            // Since we're starting near current time, current_cycle should be 1
            // Initialize with new state
            global_state initial_state{
                .initialized = true,
                .cycle_start_time = start_time,
                .last_cycle_update = start_time,
                .current_cycle = 1,  // Always start with cycle 1
                .cycle_length_sec = cycle_length_sec,
                .max_cycle_length_sec = max_cycle_length_sec
            };

            global.set(initial_state, _self);

            // Emit initialization event
            action(
                permission_level{_self, "active"_n},
                _self,
                "initevent"_n,
                std::make_tuple(cycle_length_sec, max_cycle_length_sec, start_time, initial_state.current_cycle)
            ).send();
        }
    };
}
