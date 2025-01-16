# Boid Game Rewards Smart Contract

A flexible and robust smart contract for managing game statistics, player records, and reward distributions on the Antelope blockchain.

## Overview

The Boid Game Rewards contract provides a comprehensive system for:
- Managing game configurations and statistics
- Recording and tracking player performance
- Distributing rewards based on performance metrics
- Managing time-based cycles for statistics and rewards

## Technical Architecture

### Directory Structure
```
include/
  ├── actions/
  │   ├── cycle_management.hpp    # Cycle-based time management
  │   ├── game_management.hpp     # Game state management
  │   ├── game_record.hpp         # Game statistics recording
  │   ├── init.hpp               # Contract initialization
  │   ├── record_management.hpp   # Historical records management
  │   ├── reward_management.hpp   # Reward distribution logic
  ├── tables/
  │   ├── game_tables.hpp         # Game-related table structures
  │   ├── token_tables.hpp        # Token and global state structures
  │   └── tables.hpp             # Central include point
  └── types.hpp                   # Common type definitions
```

## Tables and Data Structures

### 1. Global State
```cpp
struct global_state {
    bool initialized;
    uint32_t current_cycle;
    uint32_t cycle_length_sec;
    uint32_t max_cycle_length_sec;
    uint64_t cycle_start_time; // Time in seconds since epoch
    uint64_t last_cycle_update;
}
```
Purpose: Manages contract initialization and cycle timing

### 2. Game Configuration
```cpp
struct stat_config {
    name stat_name;       // Unique identifier for the stat
    string display_name;  // Human-readable name
    string description;   // Description of what this stat measures
    bool is_high_better; // True if higher values are better, false if lower is better
}

struct gameconfig {
    name game_name;
    string display_name;
    string metadata;
    bool active;
    vector<stat_config> stat_configs;
}
```
Purpose: Defines game configurations and their associated statistics

### 3. Player Statistics
```cpp
struct playerstats {
    uint64_t id;
    name player;
    name game_name;
    map<name, uint64_t> stats;
    uint64_t last_updated; // Time in seconds since epoch

    uint64_t primary_key() const { return id; }
    uint128_t by_player_game() const { return combine_names(player, game_name); }
    uint64_t by_game() const { return game_name.value; }
    uint64_t by_player() const { return player.value; }
}
```
Purpose: Tracks current player statistics and their latest updates

### 4. Stats History
```cpp
struct statshistory {
    uint64_t id;
    name player;
    name game_name;
    map<name, uint64_t> stats;
    uint32_t cycle_number;
    uint64_t timestamp; // Time in seconds since epoch

    uint64_t primary_key() const { return id; }
    uint128_t by_game_stat() const { return combine_names(game_name, stat_name); }
    uint128_t by_player_stat() const { return combine_names(player, stat_name); }
    uint128_t by_cycle() const { return ((uint128_t)game_name.value << 64) | cycle_number; }
}
```
Purpose: Permanent historical record of all game stats

### 5. Cycle Distribution
```cpp
struct cycledistribution {
    uint64_t id;
    name game_name;
    uint32_t cycle_number;
    name stat_name;
    asset total_reward;
    uint64_t distribution_time; // Time in seconds since epoch

    uint64_t primary_key() const { return id; }
    uint128_t by_game_cycle() const { return ((uint128_t)game_name.value << 64) | cycle_number; }
    uint64_t by_game() const { return game_name.value; }
    uint64_t by_cycle() const { return cycle_number; }
}
```
Purpose: Tracks all reward distributions and which cycles they covered

## Setup Tutorial

To get started with the Boid Game Rewards smart contract, follow these steps:

1. **Initialize the Contract**:
   - Set the initial cycle start time and cycle lengths.
   ```bash
   cleos push action contract.name initcontract '[
       1705406563,  # Start time in seconds since epoch
       604800,                 # Cycle length (7 days in seconds)
       2592000                 # Max cycle length (30 days in seconds)
   ]' -p contract.name
   ```

2. **Add and Configure Games**:
   - Add games and configure their stats.
   ```bash
   cleos push action contract.name addgame '[
       "game1",                # Game name
       "First Game",          # Display name
       "{\"description\": \"Game description\"}", # Metadata
       [{                      # Stat configs
           "stat_name": "kills",
           "display_name": "Kills",
           "description": "Number of kills",
           "is_high_better": true
       }]
   ]' -p contract.name
   ```

3. **Record Game Stats**:
   - Record player stats for completed games.
   ```bash
   cleos push action contract.name recordgame '[
       "game1",                # Game name
       "player.name",          # Player account
       {                        # Stats
           "kills": 10,
           "deaths": 5
       },
       1705407559  # Completion time in seconds since epoch
   ]' -p contract.name
   ```

4. **Distribute Rewards**:
   - Distribute rewards for a completed cycle.
   ```bash
   cleos push action contract.name distribute '[
       "game1",              # Game name
       1,                    # Cycle number
       "kills",              # Stat to rank by
       "100.0000 TOKEN",     # Total reward
       "token.contract",     # Token contract
       [50, 30, 20]         # Reward percentages
   ]' -p contract.name
   ```

## Actions and Usage

### 1. Initialization
The *initcontract* action initializes or resets the contract state:
- Authorization: Requires the contract's authority to execute.
- Validation: Ensures valid cycle lengths and start time.
- Cleanup: Clears all existing tables (global state, game configs, player stats, stats history, reward configs).
- State Setup: Initializes the contract with cycle 1, specified start time and cycle lengths.
- Event Emission: Notifies external systems about the initialization.

```bash
cleos push action contract.name initcontract '[
    1705406563,  # Start time in seconds since epoch
    604800,                 # Cycle length (7 days in seconds)
    2592000                 # Max cycle length (30 days in seconds)
]' -p contract.name
```

### 2. Game Management
The *addgame* action registers a new game in the contract:
- Authorization: Requires the contract's authority to execute.
- Validation: Ensures stat configurations are valid with non-empty names.
- Game Setup: Creates a new game configuration with specified stats.
- Reward Setup: Initializes default reward configuration for the game.

```bash
# Add a new game
cleos push action contract.name addgame '[
    "game1",                # Game name
    "First Game",          # Display name
    "{\"description\": \"Game description\"}", # Metadata
    [{                      # Stat configs
        "stat_name": "kills",
        "display_name": "Kills",
        "description": "Number of kills",
        "is_high_better": true
    }]
]' -p contract.name

# Configure an existing game
cleos push action contract.name configgame '[
    "game1",                # Game name
    "{\"description\": \"Updated description\"}"
]' -p contract.name

# Remove a game
cleos push action contract.name removegame '[
    "game1"                # Game name
]' -p contract.name
```

### 3. Recording Game Stats
The *recordgame* action:  
- Authorization: Ensures the action is authorized by the contract itself.
- Game Configuration Validation: Checks that the game exists and is active.
- Stat Validation: Ensures that the provided stats match the game's configuration.
- Cycle Determination:
 *Retrieves the current global state.
 *Validates the completion time against the current time and the contract's initialization time.
 *Determines the appropriate cycle for the game record using the determine_cycle function.
- Reward Distribution Check: Ensures that rewards have not already been distributed for the determined cycle.
- Record Storage: Saves the game record to the statshistory_table with the determined cycle number.
- Event Emission: Sends an action to notify that stats have been recorded.

```bash
# Record game stats for a player
cleos push action contract.name recordgame '[
    "game1",                # Game name
    "player.name",          # Player account
    {                        # Stats
        "kills": 10,
        "deaths": 5
    },
    1705407559  # Completion time in seconds since epoch
]' -p contract.name
```

### 4. Reward Distribution and Management
The *distribute* action:
- Authorization: Requires the contract's authority to execute.
- Validation: Ensures rewards haven't been distributed for the specified cycle.
- Stats Processing: Aggregates and ranks player stats for the cycle.
- Reward Calculation: Computes rewards based on configured percentages.
- Token Transfer: Distributes reward tokens to qualifying players.
- Event Emission: Notifies external systems about the reward distribution.

```bash
# Distribute rewards for a cycle
cleos push action contract.name distribute '[
    "game1",              # Game name
    1,                    # Cycle number
    "kills",              # Stat to rank by
    "100.0000 TOKEN",     # Total reward
    "token.contract",     # Token contract
    [50, 30, 20]         # Reward percentages
]' -p contract.name

# Find cycles missing distributions
cleos push action contract.name findmissing '[
    "game1",              # Game name
    1,                    # Start cycle
    100                   # End cycle
]' -p contract.name

# Clean up old game records
cleos push action contract.name trimhistory '[
    "game1",                          # Game name
    1705406563,           # Start time
    1705408563,           # End time
    1000                             # Max records to delete
]' -p contract.name
```

### 5. Cycle Management
The *setcyclelen* action allows for the adjustment of the cycle length in seconds:
- Authorization: Requires the contract's authority to execute.
- Validation: Checks that the contract is initialized.
- Ensures the new cycle length is greater than zero and does not exceed the maximum allowed cycle length.  
- Updates the cycle length in the global state.
- Emits an event to notify external systems about the change.

```bash
# Set a new cycle length
cleos push action contract.name setcyclelen '[
    604800                  # New cycle length in seconds (7 days)
]' -p contract.name
```

### 6. Token Management
The *settoken* action configures the reward token for the contract:
- Authorization: Requires the contract's authority to execute.
- Token Setup: Specifies the token contract account and symbol for rewards.
- State Management: Creates or updates the token configuration in the contract state.
- Flexibility: Allows changing the reward token if needed.

```bash
# Set token contract and symbol
cleos push action contract.name settoken '[
    "token.contract",      # Token contract
    "4,TOKEN"              # Token symbol
]' -p contract.name
```

### 5. Data Management
The contract provides tools for efficient data management:

1. **Historical Record Cleanup**:
   - `trimhistory`: Safely removes old game records
   - Only deletes records from cycles that have had rewards distributed
   - Configurable batch size to prevent timeouts

2. **Distribution Tracking**:
   - `findmissing`: Identifies cycles without reward distributions
   - Helps ensure no cycles are missed in reward distribution
   - Returns list of cycles needing attention

3. **Safety Mechanisms**:
   - Cannot delete records from cycles without distributions
   - Cannot record new games for cycles already distributed
   - All operations maintain data integrity

## Security Considerations

1. **Authorization**:
   - All actions require contract authority
   - Strict validation of inputs and permissions

2. **Data Integrity**:
   - Stats are validated against game configuration
   - Cycle boundaries are strictly enforced
   - Historical data is immutable

3. **Resource Management**:
   - Efficient table design with appropriate indices
   - Optimized data structures for minimal RAM usage

## Queries and Data Access

### Available Indices and Example Queries

1. Player Stats:
   - By player-game: Find all stats for a player in a specific game
     ```bash
     # Get all stats for player1 in game1
     cleos get table contract.name contract.name playerstats \
         --index 2 \
         --key-type uint128 \
         --lower "$(echo $(($(echo -n 'player1' | xxd -p -u) << 64 | $(echo -n 'game1' | xxd -p -u))))"
     ```
   
   - By game: List all stats for a game
     ```bash
     # Get all stats for game1
     cleos get table contract.name contract.name playerstats \
         --index 3 \
         --key-type name \
         --lower game1 --upper game1
     ```
   
   - By player: List all stats for a player
     ```bash
     # Get all stats for player1
     cleos get table contract.name contract.name playerstats \
         --index 4 \
         --key-type name \
         --lower player1 --upper player1
     ```
   
   - By game-cycle: Get stats for a specific game cycle
     ```bash
     # Get all stats for game1 in cycle 1
     cleos get table contract.name contract.name playerstats \
         --index 5 \
         --key-type uint128 \
         --lower "$(echo $(($(echo -n 'game1' | xxd -p-u) << 64 | 1)))"
     ```
   
   - By completion: Time-based queries
     ```bash
     # Get stats between two timestamps
     cleos get table contract.name contract.name playerstats \
         --index 6 \
         --key-type uint128 \
         --lower "$(echo $(($(date -d '2025-01-16T14:00:00' +%s) << 64)))" \
         --upper "$(echo $(($(date -d '2025-01-16T15:00:00' +%s) << 64)))"
     ```

2. Stats History:
   - By game stat: Historical stats for a game
     ```bash
     # Get all historical stats for game1
     cleos get table contract.name contract.name statshistory \
         --index 2 \
         --key-type uint128 \
         --lower "$(echo $(($(echo -n 'game1' | xxd -p-u) << 64)))"
     ```
   
   - By player stat: Historical stats for a player
     ```bash
     # Get all historical stats for player1
     cleos get table contract.name contract.name statshistory \
         --index 3 \
         --key-type uint128 \
         --lower "$(echo $(($(echo -n 'player1' | xxd -p-u) << 64)))"
     ```
   
   - By cycle: All stats from a specific cycle
     ```bash
     # Get all historical stats for cycle 1
     cleos get table contract.name contract.name statshistory \
         --index 4 \
         --key-type uint128 \
         --lower "$(echo $((1 << 64)))"
     ```

Note: These queries use the following index numbers:
- playerstats table:
  - 1: Primary key (id)
  - 2: byplayergame
  - 3: bygame
  - 4: byplayer
  - 5: bygamecycle
  - 6: bycompletion
- statshistory table:
  - 1: Primary key (id)
  - 2: bygamestat
  - 3: byplayerstat
  - 4: bycycle
