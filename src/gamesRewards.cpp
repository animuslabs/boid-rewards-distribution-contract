#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include "../include/tables/tables.hpp"
#include "../include/actions/game_management.hpp"
#include "../include/actions/record_management.hpp"
#include "../include/actions/reward_management.hpp"
#include "../include/actions/cycle_management.hpp"
#include "../include/actions/init.hpp"
#include "../include/actions/game_record.hpp"

using namespace eosio;
using namespace gamerewards;

#ifndef CONTRACT_NAME
#define CONTRACT_NAME "gamerewards"
#endif

class [[eosio::contract(CONTRACT_NAME)]] gamerewards_contract : public contract {
private:
    gamerewards::game_management game_mgmt;
    gamerewards::record_management record_mgmt;
    gamerewards::reward_management reward_mgmt;
    gamerewards::cycle_management cycle_mgmt;
    gamerewards::init init_mgmt;
    gamerewards::game_record game_record_mgmt;

public:
    gamerewards_contract(name receiver, name code, datastream<const char*> ds)
        : contract(receiver, code, ds),
          game_mgmt(receiver, code, ds),
          record_mgmt(receiver, code, ds),
          reward_mgmt(receiver, code, ds),
          cycle_mgmt(receiver, code, ds),
          init_mgmt(receiver, code, ds),
          game_record_mgmt(receiver, code, ds) {}

    // Initialization action
    [[eosio::action]]
    void initcontract(time_point_sec start_time, uint32_t cycle_length_sec, uint32_t max_cycle_length_sec) {
        init_mgmt.initcontract(start_time, cycle_length_sec, max_cycle_length_sec);
    }

    // Game management actions
    [[eosio::action]]
    void addgame(name game_name, string display_name, string metadata, vector<gamerewards::stat_config> stat_configs) {
        game_mgmt.addgame(game_name, display_name, metadata, stat_configs);
    }

    [[eosio::action]]
    void updategame(name game_name, string display_name, string metadata, vector<gamerewards::stat_config> stat_configs) {
        game_mgmt.updategame(game_name, display_name, metadata, stat_configs);
    }

    [[eosio::action]]
    void removegame(name game_name) {
        game_mgmt.removegame(game_name);
    }

    // Record management actions
    [[eosio::action]]
    void recordgame(name game_name, name player, std::map<name, uint64_t> stats) {
        time_point_sec completion_time = time_point_sec(current_time_point());
        game_record_mgmt.recordgame(game_name, player, stats, completion_time);
    }

    [[eosio::action]]
    void playerstats(name game_name, name player) {
        game_record_mgmt.playerstats(game_name, player);
    }

    // Reward management actions
    [[eosio::action]]
    void setdistconf(name game_name, name destination_contract, std::string memo_template) {
        reward_mgmt.setdistconf(game_name, destination_contract, memo_template);
    }

    [[eosio::action]]
    void distribute(name game_name, uint32_t cycle, name stat_name, asset total_reward, 
                   name destination_contract, std::vector<uint8_t> reward_percentages) {
        reward_mgmt.distribute(game_name, cycle, stat_name, total_reward, 
                             destination_contract, reward_percentages);
    }

    // Cycle management actions
    [[eosio::action]]
    void setcyclelen(uint32_t new_length_sec) {
        cycle_mgmt.setcyclelen(new_length_sec);
    }

    [[eosio::action]]
    void getcycle() {
        cycle_mgmt.getcycle();
    }
};