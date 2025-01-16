#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

namespace gamerewards {
    using namespace eosio;

    // Token transfer action
    struct token_transfer {
        name from;
        name to;
        asset quantity;
        std::string memo;

        static void send(const name& from, const name& to, const asset& quantity, 
                        const std::string& memo, const name& contract) {
            token_transfer transfer_data = {
                .from = from,
                .to = to,
                .quantity = quantity,
                .memo = memo
            };

            action(
                permission_level{from, "active"_n},
                contract,
                "transfer"_n,
                transfer_data
            ).send();
        }
    };

    // Notification action for system token deposits
    struct [[eosio::table("sysdepnotify"), eosio::contract("gamerewards")]] system_deposit_notification {
        name depositor;
        asset quantity;
        std::string message;
    };
}
