#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>
#include <map>
#include "tables/game_tables.hpp"

using namespace eosio;
using namespace std;

namespace gamerewards {
    // Struct for reward tiers
    struct reward_tiers {
        uint32_t tier;
        asset reward_amount;
        uint32_t min_score;
        uint32_t max_score;
    };
}
