#pragma once
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include "token_tables.hpp"
#include "game_tables.hpp"
#include "user_tables.hpp"
#include "../types.hpp"

// This file serves as a central include point for all table definitions
// Each specific table type is defined in its own file for better organization

namespace gamerewards {
    using namespace eosio;
}
