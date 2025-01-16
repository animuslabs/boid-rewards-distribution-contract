#pragma once
#include <eosio/eosio.hpp>

namespace gamerewards {
    using namespace eosio;

    // Configuration for how rewards should be distributed for a game
    struct [[eosio::table]] reward_distribution_config {
        uint16_t game_id;
        name destination_contract;     // Where to send the rewards (e.g., "boid")
        std::string memo_template;     // Template for memo (e.g., "deposit boid_id={player}")
        bool use_direct_transfer;      // If false, uses the destination contract

        uint64_t primary_key() const { return game_id; }
    };
    typedef multi_index<"rewarddist"_n, reward_distribution_config> reward_distribution_table;
}
