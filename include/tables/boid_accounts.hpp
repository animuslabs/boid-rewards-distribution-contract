#pragma once
#include <eosio/eosio.hpp>

namespace boid {
    using namespace eosio;

    struct [[eosio::table, eosio::contract("boid")]] account {
        name boid_id;  // Unique identifier for the Boid account

        uint64_t primary_key() const { return boid_id.value; }

        EOSLIB_SERIALIZE(account, (boid_id))
    };

    typedef eosio::multi_index<"accounts"_n, account> accounts_table;
}
