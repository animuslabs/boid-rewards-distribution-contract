#pragma once
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include "config_tables.hpp"
#include "contract_tables.hpp"

// This file serves as a central include point for all table definitions
// Each specific table type is defined in its own file for better organization

namespace gamerewards {
    using namespace eosio;
}
