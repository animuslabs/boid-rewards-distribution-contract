#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <map>
#include <vector>
#include <string>

#ifndef CONTRACT_NAME
#define CONTRACT_NAME "gamerewards"
#endif

namespace gamerewards {
    using namespace eosio;
    using std::string;
    using std::vector;
    using std::map;

    // Centralized table for player data
    struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] players {
        eosio::name player;                // Unique identifier for the user
        std::string metadata;             // Additional user-related metadata
        uint32_t games_played = 0;  // Total number of games played by the player

        uint64_t primary_key() const { return player.value; }
    };
    typedef eosio::multi_index<"players"_n, players> players_table;

    struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] gamerecords {
        uint64_t id;
        name player;
        uint8_t game_id;
        std::vector<eosio::name> stats_names; // Names of the stats
        std::vector<uint64_t> stats_values;  // Corresponding values for the stats
        uint32_t cycle_number;       // Cycle number during which the game was completed
        bool rewards_distributed = false;
        time_point_sec game_completion_time;  // When the game was actually completed
        time_point_sec last_updated;         // Last time the record was updated

        uint64_t primary_key() const { return id; }
        uint64_t by_game() const { return game_id; }
        uint64_t by_player() const { return player.value; }
        uint128_t by_player_game() const {
            return ((uint128_t)player.value << 64) | game_id;
        }
        uint128_t by_game_cycle() const {
            return ((uint128_t)game_id << 64) | cycle_number;
        }
        uint128_t by_completion() const {
            return ((uint128_t)game_completion_time.sec_since_epoch() << 64) | id;
        }
    };

    using gamerecords_table = eosio::multi_index<"gamerecords"_n, gamerecords,
        indexed_by<"byplayergame"_n, const_mem_fun<gamerecords, uint128_t, &gamerecords::by_player_game>>,
        indexed_by<"bygame"_n, const_mem_fun<gamerecords, uint64_t, &gamerecords::by_game>>,
        indexed_by<"byplayer"_n, const_mem_fun<gamerecords, uint64_t, &gamerecords::by_player>>,
        indexed_by<"bygamecycle"_n, const_mem_fun<gamerecords, uint128_t, &gamerecords::by_game_cycle>>,
        indexed_by<"bycompletion"_n, const_mem_fun<gamerecords, uint128_t, &gamerecords::by_completion>>
    >;

    struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] rewardsrecorded {
        uint64_t id;                     // Unique record ID
        uint8_t game_id;                 // Game ID
        uint32_t cycle_number;           // Cycle number
        eosio::name stat_name;                  // Stat name used for rewards
        eosio::asset total_reward;              // Total reward distributed
        std::vector<name> rewarded_players;   // List of rewarded players
        eosio::time_point_sec distribution_time;// Distribution timestamp

        uint64_t primary_key() const { return id; }
        uint128_t by_game_cycle() const {
            return ((uint128_t)game_id << 64) | cycle_number;
        }
    };
    using rewardsrecorded_table = eosio::multi_index<"rewardsrec"_n, rewardsrecorded,
        indexed_by<"bygamecycle"_n, const_mem_fun<rewardsrecorded, uint128_t, &rewardsrecorded::by_game_cycle>>
    >;
}
