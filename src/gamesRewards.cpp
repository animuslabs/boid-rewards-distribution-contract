#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include "../include/tables/tables.hpp"
#include "../include/actions/actions.hpp"
#include <vector>
#include <string>

using namespace eosio;
using namespace gamerewards;
using std::string;
using std::vector;

#ifndef CONTRACT_NAME
#define CONTRACT_NAME gamerewards
#endif

class [[eosio::contract(CONTRACT_NAME)]] gamerewards_contract : public contract {
private:
    gamerewards::game_management game_mgmt;
    gamerewards::reward_management reward_mgmt;
    gamerewards::init init_mgmt;
    gamerewards::game_record game_record_mgmt;

public:
    gamerewards_contract(name receiver, name code, datastream<const char*> ds)
        : contract(receiver, code, ds),
          game_mgmt(receiver, code, ds),
          reward_mgmt(receiver, code, ds),
          init_mgmt(receiver, code, ds),
          game_record_mgmt(receiver, code, ds) {}

    // Initialization action
    [[eosio::action]]
    void initcontract(time_point_sec start_time, uint32_t cycle_length_sec, uint32_t max_cycle_length_sec, 
                      uint8_t max_reward_tiers, uint8_t min_reward_percentage) {
        init_mgmt.initcontract(start_time, cycle_length_sec, max_cycle_length_sec, max_reward_tiers, min_reward_percentage);
    }

    // Game management actions
    [[eosio::action]]
    void setgame(uint8_t game_id, string display_name, string metadata, std::vector<eosio::name> stat_names) {
        game_mgmt.setgame(game_id, display_name, metadata, stat_names);
    }

    [[eosio::action]]
    void removegame(uint8_t game_id) {
        game_mgmt.removegame(game_id);
    }

    // Record management actions
    [[eosio::action]]
    void recordgame(std::vector<gamerewards::game_record_data> records) {
        game_record_mgmt.recordgame(records);
    }

    [[eosio::action]]
    void clearrecord(const std::vector<uint64_t>& record_ids) {
        game_record_mgmt.clearrecord(record_ids);
    }

    // Reward management actions
    [[eosio::action]]
    void setdistconf(uint8_t game_id, name destination_contract, std::string memo_template, bool use_direct_transfer = true) {
        reward_mgmt.setdistconf(game_id, destination_contract, memo_template, use_direct_transfer);
    }

    [[eosio::action]]
    void distribute(uint8_t game_id, uint32_t cycle, name stat_name, asset total_reward, 
                    name token_contract, std::vector<uint8_t> reward_percentages) {
        reward_mgmt.distribute(game_id, cycle, stat_name, total_reward, token_contract, reward_percentages);
    }

    // Token management actions
    [[eosio::action]]
    void settoken(name token_contract, symbol token_symbol, bool enabled = true) {
        reward_mgmt.settoken(token_contract, token_symbol, enabled);
    }

    [[eosio::action]]
    void removetoken(symbol token_symbol) {
        reward_mgmt.removetoken(token_symbol);
    }
};
