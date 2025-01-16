#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include "../tables/tables.hpp"

namespace gamerewards {
    using namespace eosio;

    class [[eosio::contract("gamerewards")]] token_management : public contract {
    public:
        token_management(name receiver, name code, datastream<const char*> ds)
            : contract(receiver, code, ds) {}

        [[eosio::action]]
        void settoken(name token_contract, symbol token_symbol) {
            require_auth(_self);

            // Update or create token config
            token_config_table configs(_self, _self.value);
            auto config = configs.find(token_symbol.raw());

            if (config == configs.end()) {
                configs.emplace(_self, [&](auto& c) {
                    c.token_contract = token_contract;
                    c.token_symbol = token_symbol;
                });
            } else {
                configs.modify(config, _self, [&](auto& c) {
                    c.token_contract = token_contract;
                });
            }
        }

        [[eosio::action]]
        void removetoken(symbol token_symbol) {
            require_auth(_self);

            token_config_table configs(_self, _self.value);
            auto config = configs.find(token_symbol.raw());
            check(config != configs.end(), "Token configuration not found");
            configs.erase(config);
        }
    };
}
