#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>

#ifndef CONTRACT_NAME
#define CONTRACT_NAME "gamerewards"
#endif

namespace gamerewards {
    using namespace eosio;

    // Global state singleton
    struct [[eosio::table("global"), eosio::contract(CONTRACT_NAME)]] global_state {
        bool initialized = false;
        time_point_sec cycle_start_time;
        time_point_sec last_cycle_update;  // Added for cycle management
        uint32_t last_consolidated_cycle = 0;
        uint32_t current_cycle = 0;
        uint32_t cycle_length_sec = 604800; // Default to 7 days in seconds
        uint32_t max_cycle_length_sec = 2592000; // Default to 30 days in seconds

        uint32_t get_current_cycle() const {
            if (!initialized) return 0;
            auto now = time_point_sec(current_time_point());
            return ((now.sec_since_epoch() - cycle_start_time.sec_since_epoch()) / cycle_length_sec) + 1;
        }

        time_point_sec get_cycle_start(uint32_t cycle) const {
            if (cycle <= 0) return time_point_sec(0);
            return time_point_sec(cycle_start_time.sec_since_epoch() + (cycle - 1) * cycle_length_sec);
        }

        time_point_sec get_cycle_end(uint32_t cycle) const {
            if (cycle <= 0) return time_point_sec(0);
            return time_point_sec(get_cycle_start(cycle).sec_since_epoch() + cycle_length_sec);
        }

        bool is_valid_cycle(uint32_t cycle) const {
            if (!initialized || cycle <= 0) return false;
            auto now = time_point_sec(current_time_point());
            return cycle <= get_current_cycle() && now >= get_cycle_start(cycle);
        }

        EOSLIB_SERIALIZE(global_state, (initialized)(cycle_start_time)(last_cycle_update)(last_consolidated_cycle)(current_cycle)(cycle_length_sec)(max_cycle_length_sec))
    };

    typedef eosio::singleton<"global"_n, global_state> global_singleton;
    typedef eosio::multi_index<"global"_n, global_state> global_table;

    // Token configuration for reward distribution
    struct [[eosio::table("tokenconfig"), eosio::contract(CONTRACT_NAME)]] token_config {
        name token_contract;
        symbol token_symbol;
        
        uint64_t primary_key() const { return token_symbol.raw(); }
    };

    typedef eosio::multi_index<"tokenconfig"_n, token_config> token_config_table;
}
