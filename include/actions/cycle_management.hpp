#pragma once
#include <eosio/eosio.hpp>
#include "../tables/tables.hpp"

#ifndef CONTRACT_NAME
#define CONTRACT_NAME "gamerewards"
#endif

namespace gamerewards {
    using namespace eosio;

    class [[eosio::contract(CONTRACT_NAME)]] cycle_management : public contract {
    private:
        global_singleton globals;

        void update_player_stats_for_cycle(uint32_t new_cycle) {
            playerstats_table player_stats(_self, _self.value);
            auto player_it = player_stats.begin();
            while (player_it != player_stats.end()) {
                if (player_it->cycle_number != new_cycle) {
                    player_stats.modify(player_it, _self, [&](auto& p) {
                        p.cycle_number = new_cycle;
                        p.stats.clear();
                        p.rewards_distributed = false;
                    });
                }
                player_it++;
            }
        }

        bool is_cycle_boundary(const time_point_sec& current_time, const global_state& state) {
            uint32_t elapsed = current_time.sec_since_epoch() - state.last_cycle_update.sec_since_epoch();
            return elapsed >= state.cycle_length_sec;
        }

    public:
        cycle_management(name receiver, name code, datastream<const char*> ds) 
            : contract(receiver, code, ds), globals(receiver, receiver.value) {}

        [[eosio::action]]
        void setcyclelen(uint32_t new_length_sec) {
            require_auth(_self);
            check(globals.exists(), "Contract not initialized");
            auto state = globals.get();

            check(new_length_sec > 0, "Cycle length must be greater than 0");
            check(new_length_sec <= state.max_cycle_length_sec, "Cycle length cannot exceed maximum cycle length");

            // Get current time
            time_point_sec current_time = time_point_sec(current_time_point());
            
            // Calculate current progress in the cycle
            uint32_t elapsed_in_cycle = current_time.sec_since_epoch() - state.last_cycle_update.sec_since_epoch();
            
            // If we're shortening the cycle and we've exceeded the new length, increment cycle
            if (new_length_sec < state.cycle_length_sec && elapsed_in_cycle >= new_length_sec) {
                state.current_cycle++;
                state.last_cycle_update = current_time;
                
                // Update player stats for the new cycle
                update_player_stats_for_cycle(state.current_cycle);
            }
            
            state.cycle_length_sec = new_length_sec;
            globals.set(state, _self);
        }
    };
}
