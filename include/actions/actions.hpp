#pragma once
#include "init.hpp"
#include "reward_management.hpp"
#include "game_record.hpp"
#include "game_management.hpp"
#include "./../tables/tables.hpp"  // Include tables if actions depend on table structures

// This file serves as a central include point for all action definitions
// Each specific action type is defined in its own file for better organization

namespace gamerewards {
    using namespace eosio;
}
