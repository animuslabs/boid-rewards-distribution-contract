// this one is done

#pragma once
#include <eosio/eosio.hpp>
#include "../tables/tables.hpp"
#include <vector>
#include <string>

#ifndef CONTRACT_NAME
#define CONTRACT_NAME "gamerewards"
#endif

namespace gamerewards {
    using namespace eosio;
    using std::string;
    using std::vector;

    struct game_record_data {
        uint8_t game_id;
        name player;
        vector<name> stats_names;      // Stat names (e.g., ["kills", "score"])
        vector<uint64_t> stats_values; // Corresponding values for the stats
        time_point_sec completion_time;

        EOSLIB_SERIALIZE(game_record_data, (game_id)(player)(stats_names)(stats_values)(completion_time))
    };

    class [[eosio::contract(CONTRACT_NAME)]] game_record : public contract {
    private:
        global_singleton globals{_self, _self.value};

        uint32_t determine_cycle(time_point_sec completion_time) {
            check(globals.exists(), "Contract not initialized");
            auto global = globals.get();

            check(global.cycle_length_sec > 0, "Cycle length must be greater than zero");
            check(completion_time <= current_time_point(), "Game completion time cannot be in the future");
            check(completion_time >= global.cycles_initiation_time, "Game completion time cannot be before contract initialization");

            uint32_t elapsed_sec = completion_time.sec_since_epoch() - global.cycles_initiation_time.sec_since_epoch();
            return (elapsed_sec / global.cycle_length_sec) + 1;
        }

    public:
        game_record(name receiver, name code, datastream<const char*> ds)
            : contract(receiver, code, ds), globals(_self, _self.value) {}

        [[eosio::action]]
        void recordgame(vector<game_record_data> records) {
            require_auth(_self);

            check(!records.empty(), "No records provided");
            check(records.size() <= 100, "Too many records in a single transaction! Max: 100");

            players_table players(_self, _self.value);
            gamerecords_table gamerecords(_self, _self.value);
            gameconfig_table gameconfigs(_self, _self.value);

            for (const auto& record : records) {
                // Ensure player exists or create a new one
                auto player_itr = players.find(record.player.value);
                if (player_itr == players.end()) {
                    players.emplace(_self, [&](auto& row) {
                        row.player = record.player;
                        row.games_played = 0;
                        row.metadata = ""; // Default metadata
                    });
                    player_itr = players.find(record.player.value);
                }

                // Validate game ID
                auto game_itr = gameconfigs.find(record.game_id);
                check(game_itr != gameconfigs.end(), "Game ID not found: " + std::to_string(record.game_id));

                // Validate stats
                const auto& valid_stats = game_itr->stats_names;
                check(record.stats_names.size() == record.stats_values.size(),
                      "Mismatched stats_names and stats_values sizes");
                for (const auto& stat_name : record.stats_names) {
                    check(std::find(valid_stats.begin(), valid_stats.end(), stat_name) != valid_stats.end(),
                          "Invalid stat name: " + stat_name.to_string());
                }

                // Calculate the cycle for the record
                uint32_t cycle_number = determine_cycle(record.completion_time);

                // Record game stats
                gamerecords.emplace(_self, [&](auto& row) {
                    row.id = gamerecords.available_primary_key();
                    row.player = record.player;
                    row.game_id = record.game_id;
                    row.stats_names = record.stats_names;
                    row.stats_values = record.stats_values;
                    row.cycle_number = cycle_number;
                    row.game_completion_time = record.completion_time;
                    row.last_updated = current_time_point();
                    row.rewards_distributed = false;
                });

                // Update player stats
                players.modify(player_itr, _self, [&](auto& row) {
                    row.games_played++;
                });
            }
        }

        [[eosio::action]]
        void clearrecord(std::optional<uint32_t> cycle_number = std::nullopt,
                        std::optional<time_point_sec> start_time = std::nullopt,
                        std::optional<time_point_sec> end_time = std::nullopt,
                        std::optional<bool> rewards_distributed = std::nullopt,
                        uint64_t max_deletions = 1000,
                        bool return_only = false) {
            require_auth(_self);

            gamerecords_table gamerecords(_self, _self.value);
            auto idx = gamerecords.get_index<"bygamecycle"_n>();
            std::vector<gamerecords_table::const_iterator> matched_records;
            uint64_t deletions = 0;

            for (auto it = idx.begin(); it != idx.end() && deletions < max_deletions;) {
                // Check if the current record matches the filters
                if ((cycle_number && it->cycle_number != *cycle_number) ||
                    (start_time && it->game_completion_time < *start_time) ||
                    (end_time && it->game_completion_time > *end_time) ||
                    (rewards_distributed && it->rewards_distributed != *rewards_distributed)) {
                    ++it; // Skip non-matching record
                    continue;
                }

                // Either collect matching records or delete them
                if (return_only) {
                    // Log or process matching records as needed
                    ++it; // Move to the next record
                } else {
                    it = idx.erase(it); // Delete matching record
                    ++deletions;
                }
            }

            if (return_only) {
                check(!matched_records.empty(), "No matching records found");
                for (const auto& record : matched_records) {
                    print_f("Matched Record: ID: %, Player: %, Game: %, Cycle: %\n",
                            record->id, record->player, record->game_id, record->cycle_number);
                }
            }

        }

        [[eosio::action]]
        void updaterecord(
                          uint64_t record_id,
                          std::optional<vector<name>> updated_stats_names = std::nullopt,
                          std::optional<vector<uint64_t>> updated_stats_values = std::nullopt,
                          std::optional<time_point_sec> updated_completion_time = std::nullopt,
                          std::optional<bool> rewards_distributed = std::nullopt) {
            require_auth(_self);

            gamerecords_table gamerecords(_self, _self.value);

            // Find the record by ID
            auto record_itr = gamerecords.find(record_id);
            check(record_itr != gamerecords.end(), "Game record not found with ID: " + std::to_string(record_id));

            // Modify the existing record
            gamerecords.modify(record_itr, _self, [&](auto& row) {
                if (updated_stats_names && updated_stats_values) {
                    check(updated_stats_names->size() == updated_stats_values->size(),
                          "Mismatched updated_stats_names and updated_stats_values sizes");
                    row.stats_names = *updated_stats_names;
                    row.stats_values = *updated_stats_values;
                }
                // Update completion time if provided
                if (updated_completion_time) {
                    row.game_completion_time = *updated_completion_time;
                    row.cycle_number = determine_cycle(*updated_completion_time); // Recalculate cycle
                }

                // Update rewards distribution status if provided
                if (rewards_distributed) {
                    row.rewards_distributed = *rewards_distributed;
                }

                // Update the last modified timestamp
                row.last_updated = current_time_point();
            });
        }
    };
}