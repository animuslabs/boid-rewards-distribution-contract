#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

namespace gamerewards {
    using namespace eosio;

    // Table for user statistics
    struct [[eosio::table("userstat")]] user_stat {
        name boid_id;
        uint32_t games_played;
        uint32_t games_won;
        asset total_rewards;
        uint64_t total_score;
        std::string metadata;

        uint64_t primary_key() const { return boid_id.value; }
    };
    typedef multi_index<"userstat"_n, user_stat> user_stat_table;

    // Table for user game history
    struct [[eosio::table("gamehistory")]] game_history {
        uint64_t history_id;
        name boid_id;
        uint64_t record_id;
        uint64_t score;
        asset reward;
        uint64_t timestamp;

        uint64_t primary_key() const { return history_id; }
        uint64_t by_user() const { return boid_id.value; }
        uint64_t by_record() const { return record_id; }
    };
    typedef multi_index<
        "gamehistory"_n, 
        game_history,
        indexed_by<"byuser"_n, const_mem_fun<game_history, uint64_t, &game_history::by_user>>,
        indexed_by<"byrecord"_n, const_mem_fun<game_history, uint64_t, &game_history::by_record>>
    > game_history_table;
}
