#pragma once
#include <eosio/eosio.hpp>
#include "../tables/tables.hpp"

#ifndef CONTRACT_NAME
#define CONTRACT_NAME "gamerewards"
#endif

namespace gamerewards {
    using namespace eosio;

    class [[eosio::contract(CONTRACT_NAME)]] init : public contract {
    public:
        init(name receiver, name code, datastream<const char*> ds) 
            : contract(receiver, code, ds) {}

        /**
         * Initialize the contract with global configuration.
         *
         * @param start_time The start time for the cycles.
         * @param cycle_length_sec The length of each cycle in seconds.
         * @param max_cycle_length_sec The maximum allowed cycle length in seconds.
         * @param max_reward_tiers The maximum number of reward tiers.
         * @param min_reward_percentage The minimum percentage of rewards per tier.
         */
        [[eosio::action]]
        void initcontract(time_point_sec start_time, uint32_t cycle_length_sec, uint32_t max_cycle_length_sec, 
                          uint8_t max_reward_tiers, uint8_t min_reward_percentage) {
            require_auth(_self);

            // Validate inputs
            check(cycle_length_sec > 0, "Cycle length must be greater than 0");
            check(max_cycle_length_sec > 0, "Maximum cycle length must be greater than 0");
            check(max_cycle_length_sec >= cycle_length_sec, "Maximum cycle length cannot be less than cycle length");
            check(start_time.sec_since_epoch() > 0, "Invalid start time");

            check(max_reward_tiers > 0, "Maximum reward tiers must be greater than 0");
            check(min_reward_percentage > 0, "Minimum reward percentage must be greater than 0");

            auto now = time_point_sec(current_time_point());
            constexpr uint32_t MAX_FUTURE_SECONDS = 7 * 24 * 60 * 60; // 7 days in seconds

            check(start_time.sec_since_epoch() >= now.sec_since_epoch() - 60, 
                  "Start time cannot be in the past");
            check(start_time.sec_since_epoch() <= now.sec_since_epoch() + MAX_FUTURE_SECONDS, 
                  "Start time cannot be more than 7 days in the future");

            // Clear all tables
            clear_table<global_singleton>(_self);
            clear_table<tokenconfig_table>(_self);
            clear_table<gamerecords_table>(_self);        
            clear_table<rewardsrecorded_table>(_self);
            clear_table<rewarddistconfig_table>(_self);
            clear_table<gameconfig_table>(_self);

            // Initialize the global state
            global_singleton globals(_self, _self.value);
            globalconfig initial_state{
                .initialized = true,
                .cycles_initiation_time = start_time,
                .cycle_length_sec = cycle_length_sec,
                .max_cycle_length_sec = max_cycle_length_sec,
                .max_reward_tiers = max_reward_tiers,
                .min_reward_percentage = min_reward_percentage
            };

            globals.set(initial_state, _self);
        }

        template<typename Table>
        void clear_table(name scope) {
            if constexpr (std::is_same_v<Table, global_singleton>) {
                // Handle singleton
                Table table(_self, scope.value);
                if (table.exists()) {
                    table.remove();
                }
            } else {
                // Handle multi_index tables
                Table table(_self, scope.value);
                auto itr = table.begin();
                while (itr != table.end()) {
                    itr = table.erase(itr);
                }
            }
        }
    };
}
