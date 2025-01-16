#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>

#ifndef CONTRACT_NAME
#define CONTRACT_NAME "gamerewards"
#endif

namespace gamerewards {
    using namespace eosio;
    using std::string;
    using std::vector;
    using std::map;

    struct stat_config {
        name stat_name;
        string display_name;
        string description;
        bool is_high_better;
    };

    struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] gameconfig {
        name game_name;
        string display_name;
        string metadata;
        bool active = true;
        vector<stat_config> stat_configs;

        uint64_t primary_key() const { return game_name.value; }
    };

    struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] playerstats {
        uint64_t id;
        name boid_id;
        name game_name;
        map<name, uint64_t> stats;  // Combined stats map
        uint32_t cycle_number;      // This will be determined by the game completion time
        bool rewards_distributed = false;
        time_point_sec game_completion_time;  // When the game was actually completed
        time_point_sec last_updated;

        uint64_t primary_key() const { return id; }
        uint64_t by_game() const { return game_name.value; }
        uint64_t by_player() const { return boid_id.value; }
        uint128_t by_player_game() const {
            return ((uint128_t)boid_id.value << 64) | game_name.value;
        }
        uint128_t by_game_cycle() const {
            return ((uint128_t)game_name.value << 64) | cycle_number;
        }
        uint128_t by_completion() const {
            return ((uint128_t)game_completion_time.sec_since_epoch() << 64) | id;
        }
    };

    struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] statshistory {
        uint64_t id;
        name boid_id;
        name game_name;
        map<name, uint64_t> stats;
        uint32_t cycle_number;
        time_point_sec timestamp;

        uint64_t primary_key() const { return id; }
        uint128_t by_game_stat() const {
            return ((uint128_t)game_name.value << 64) | id;
        }
        uint128_t by_player_stat() const {
            return ((uint128_t)boid_id.value << 64) | id;
        }
        uint128_t by_cycle() const {
            return ((uint128_t)cycle_number << 64) | id;
        }
        uint128_t by_game_cycle() const {
            return ((uint128_t)game_name.value << 64) | cycle_number;
        }
    };

    struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] reward_distribution_config {
        name game_name;
        uint32_t min_players;
        uint32_t max_players;
        uint32_t cycle_length_sec;
        name destination_contract;
        string memo_template;

        uint64_t primary_key() const { return game_name.value; }
    };

    struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] rewardconfig {
        name game_name;
        asset total_reward_pool;
        uint32_t start_cycle;
        uint32_t end_cycle;
        uint32_t last_distributed_cycle;
        vector<uint8_t> reward_percentages;

        uint64_t primary_key() const { return game_name.value; }
    };

    struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] cycledistribution {
        uint64_t id;
        name game_name;
        uint32_t cycle_number;
        name stat_name;
        asset total_reward;
        time_point_sec distribution_time;

        uint64_t primary_key() const { return id; }
        uint128_t by_game_cycle() const {
            return ((uint128_t)game_name.value << 64) | cycle_number;
        }
        uint64_t by_game() const { return game_name.value; }
        uint64_t by_cycle() const { return cycle_number; }
    };

    using gameconfig_table = eosio::multi_index<"gameconfigs"_n, gameconfig>;

    using playerstats_table = eosio::multi_index<"playerstats"_n, playerstats,
        indexed_by<"byplayergame"_n, const_mem_fun<playerstats, uint128_t, &playerstats::by_player_game>>,
        indexed_by<"bygame"_n, const_mem_fun<playerstats, uint64_t, &playerstats::by_game>>,
        indexed_by<"byplayer"_n, const_mem_fun<playerstats, uint64_t, &playerstats::by_player>>,
        indexed_by<"bygamecycle"_n, const_mem_fun<playerstats, uint128_t, &playerstats::by_game_cycle>>,
        indexed_by<"bycompletion"_n, const_mem_fun<playerstats, uint128_t, &playerstats::by_completion>>
    >;

    using statshistory_table = eosio::multi_index<"statshistory"_n, statshistory,
        indexed_by<"bygamestat"_n, const_mem_fun<statshistory, uint128_t, &statshistory::by_game_stat>>,
        indexed_by<"byplayerstat"_n, const_mem_fun<statshistory, uint128_t, &statshistory::by_player_stat>>,
        indexed_by<"bycycle"_n, const_mem_fun<statshistory, uint128_t, &statshistory::by_cycle>>,
        indexed_by<"bygamecycle"_n, const_mem_fun<statshistory, uint128_t, &statshistory::by_game_cycle>>
    >;

    using cycledistribution_table = eosio::multi_index<"cycledist"_n, cycledistribution,
        indexed_by<"bygamecycle"_n, const_mem_fun<cycledistribution, uint128_t, &cycledistribution::by_game_cycle>>,
        indexed_by<"bygame"_n, const_mem_fun<cycledistribution, uint64_t, &cycledistribution::by_game>>,
        indexed_by<"bycycle"_n, const_mem_fun<cycledistribution, uint64_t, &cycledistribution::by_cycle>>
    >;

    using reward_distribution_table = eosio::multi_index<"rewarddist"_n, reward_distribution_config>;
    using rewardconfig_table = eosio::multi_index<"rewardconfig"_n, rewardconfig>;
}
