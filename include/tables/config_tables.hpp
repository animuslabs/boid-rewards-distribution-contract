#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>
#include <vector>
#include <string>

#ifndef CONTRACT_NAME
#define CONTRACT_NAME "gamerewards"
#endif

namespace gamerewards {
    using namespace eosio;

    // Global state singleton
    struct [[eosio::table("globalconfig"), eosio::contract(CONTRACT_NAME)]] globalconfig {
        bool initialized = false;
        eosio::time_point_sec cycles_initiation_time;
        uint32_t cycle_length_sec = 604800; // Default to 7 days in seconds
        uint32_t max_cycle_length_sec = 2592000; // Default to 30 days in seconds
        uint8_t max_reward_tiers = 10;
        uint8_t min_reward_percentage = 1;

        uint32_t get_current_cycle() const {
            if (!initialized || cycle_length_sec == 0) return 0;
            auto now = time_point_sec(current_time_point());
            return ((now.sec_since_epoch() - cycles_initiation_time.sec_since_epoch()) / cycle_length_sec) + 1;
        }

        time_point_sec get_cycle_start(uint32_t cycle) const {
            if (cycle <= 0) return time_point_sec(0);
            return time_point_sec(cycles_initiation_time.sec_since_epoch() + (cycle - 1) * cycle_length_sec);
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

        EOSLIB_SERIALIZE(globalconfig,(initialized)(cycles_initiation_time)(cycle_length_sec)(max_cycle_length_sec)(max_reward_tiers)(min_reward_percentage))
    };
    typedef eosio::singleton<"globalconfig"_n, globalconfig> global_singleton;

    // Token configuration for reward distribution
    struct [[eosio::table("tokenconfig"), eosio::contract(CONTRACT_NAME)]] tokenconfig {
        eosio::name token_contract;
        eosio::symbol token_symbol;
        bool enabled = false;  // Default to disabled
        
        uint64_t primary_key() const { return token_symbol.raw(); }
    };
    typedef eosio::multi_index<"tokenconfig"_n, tokenconfig> tokenconfig_table;


        // Configuration for how rewards should be distributed for a game
    struct [[eosio::table]] rewarddistconfig {
        uint8_t game_id;
        eosio::name destination_contract;     // Where to send the rewards (e.g., "boid")
        std::string memo_template;     // Template for memo (e.g., "deposit boid_id={player}")
        bool use_direct_transfer;      // If false, uses the destination contract
        
        uint64_t primary_key() const { return game_id; } 
    };
    typedef eosio::multi_index<"rewarddist"_n, rewarddistconfig> rewarddistconfig_table;

    struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] gameconfig {
        uint8_t game_id;
        std::string display_name;
        std::string metadata;
        bool active = true;
        std::vector<eosio::name> stats_names; // Names of the stats for this game

        uint64_t primary_key() const { return game_id; }
    };
    typedef eosio::multi_index<"gameconfig"_n, gameconfig> gameconfig_table;

}
