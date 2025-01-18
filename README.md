# Boid Game Rewards Smart Contract
## Overview
The **Boid Rewards Distribution Contract** provides:
- Initialization of the contract for reward distribution cycles.
- Management of game configurations, including statistics tracking.
- Recording of player game activities and achievements.
- Distribution of rewards to players based on performance and predefined configurations.
- Token and distribution configuration management.

## Technical Architecture
### Directory Structure
```
include/
  ├── actions/
  │   ├── actions.hpp           # Central include file for all actions
  │   ├── game_management.hpp   # Game state and configuration management
  │   ├── game_record.hpp       # Game record handling and filtering
  │   ├── init.hpp             # Contract initialization and table management
  │   └── reward_management.hpp # Reward distribution and calculations
  ├── tables/
  │   ├── config_tables.hpp     # Configuration table definitions
  │   ├── contract_tables.hpp   # Game and reward table definitions
  │   └── tables.hpp           # Central include file for all tables
```

## Table Structures
---
### 1. `globalconfig` Table

#### Purpose
This table serves as the global state of the contract, managing configuration settings for reward cycles.

#### Key Fields
- **initialized**: Boolean flag indicating if the contract has been initialized
- **cycles_initiation_time**: The starting point for calculating reward cycles
- **cycle_length_sec**: The duration of each reward cycle in seconds (default: 7 days)
- **max_cycle_length_sec**: The maximum allowed length of a reward cycle (default: 30 days)
- **max_reward_tiers**: The maximum number of reward tiers allowed
- **min_reward_percentage**: The minimum percentage of total rewards allocated to a single tier

#### Key Features
- Calculates the current reward cycle using `get_current_cycle()`
- Determines the start and end times of specific cycles using `get_cycle_start()` and `get_cycle_end()`
- Validates if a cycle is active using `is_valid_cycle()`

#### Usage
This table is used to ensure that all reward distribution and game operations align with the defined reward cycle configuration.

---
### 2. `tokenconfig` Table

#### Purpose
This table manages the configuration of tokens used for rewards.

#### Key Fields
- **token_contract**: The contract responsible for managing the token
- **token_symbol**: The symbol of the token (e.g., "TLOS")
- **enabled**: A flag indicating if the token is available for distribution

#### Key Features
- Tokens can be enabled or disabled for rewards via the `enabled` field
- Ensures that only approved tokens are used for distribution

#### Usage
This table allows administrators to control which tokens are allowed for reward distribution, preventing unauthorized or unsupported tokens.

---
### 3. `rewarddistconfig` Table

#### Purpose
This table defines how rewards should be distributed for specific games.

#### Key Fields
- **game_id**: Unique identifier for the game
- **destination_contract**: The contract to which rewards are sent (e.g., "boid")
- **memo_template**: A template string for memos attached to transfers (e.g., "deposit boid_id={player}")
- **use_direct_transfer**: Indicates whether direct token transfers are used. If false, a destination contract is invoked instead

#### Key Features
- Links each game to a specific reward distribution method
- Provides flexibility for using direct transfers or contract-mediated transfers

#### Usage
This table allows game administrators to configure reward distribution mechanics for each game individually.

---
### 4. `players` Table

#### Purpose
This table tracks data about individual players, including their metadata and performance.

#### Key Fields
- **player**: The unique identifier for the player (EOSIO account name)
- **metadata**: Additional metadata about the player (e.g., profile details)
- **games_played**: The total number of games the player has participated in

#### Key Features
- Allows the contract to maintain a record of all participating players
- Tracks how many games each player has played

#### Usage
This table is used for managing player-related information and querying player-specific data for actions such as reward distribution or performance tracking.

---
### 5. `gamerecords` Table

#### Purpose
This table stores detailed records of games played by players, including their performance and associated statistics.

#### Key Fields
- **id**: Unique identifier for the game record
- **player**: The player who participated in the game
- **game_id**: Identifier for the specific game
- **stats_names**: A list of statistics tracked for the game (e.g., kills, score)
- **stats_values**: Corresponding values for the statistics (e.g., 10 kills, 500 points)
- **cycle_number**: The reward cycle during which the game was completed
- **rewards_distributed**: Boolean indicating if rewards for this record have been distributed
- **game_completion_time**: The time when the game was completed
- **last_updated**: Timestamp of the last update to this record

#### Indexes
- **by_game**: Indexes records by the game ID
- **by_player**: Indexes records by the player
- **by_game_cycle**: Indexes records by a combination of game ID and cycle number

#### Key Features
- Allows querying and filtering records by player, game, or reward cycle
- Tracks the progress of rewards distribution for each record

#### Usage
This table is used to log and retrieve game-related data, ensuring accurate reward calculations and performance tracking.

---
### 6. `rewardsrecorded` Table

#### Purpose
This table logs information about rewards distributed for specific games and cycles.

#### Key Fields
- **id**: Unique identifier for the reward record
- **game_id**: Identifier for the specific game
- **cycle_number**: The reward cycle during which the rewards were distributed
- **stat_name**: The name of the statistic used for calculating rewards (e.g., kills, score)
- **total_reward**: The total reward amount distributed for the game and cycle
- **rewarded_players**: A list of players who received rewards
- **distribution_time**: The time when the rewards were distributed

#### Indexes
- **by_game_cycle**: Indexes records by a combination of game ID and cycle number

#### Key Features
- Tracks which players received rewards and for which games and cycles
- Provides a historical record of rewards distribution

#### Usage
This table is used for auditing and verifying reward distributions, as well as providing transparency to players and administrators.

## Actions

### `initcontract`

Initializes the contract with global configuration and clears all tables.

#### Parameters:
- `start_time` (time_point_sec): Start time for the cycles
- `cycle_length_sec` (uint32_t): Length of each cycle in seconds
- `max_cycle_length_sec` (uint32_t): Maximum allowed cycle length in seconds
- `max_reward_tiers` (uint8_t): Maximum number of reward tiers
- `min_reward_percentage` (uint8_t): Minimum percentage of rewards per tier

#### Example:
```bash
cleos push action gamerewards initcontract '["2025-01-01T00:00:00", 604800, 2592000, 10, 5]' -p gamerewards@active
```

### `setgame`

Adds or updates a game configuration.

#### Parameters:
- `game_id` (uint8_t): Unique identifier for the game
- `display_name` (string): Display name for the game
- `metadata` (string): Additional metadata for the game
- `stats_names` (vector<name>): List of unique stat names for the game

#### Example:
```bash
cleos push action gamerewards setgame '[1, "Awesome Game", "Some metadata", ["kills", "score"]]' -p gamerewards@active
```

### `removegame`

Removes a game configuration by its unique ID.

#### Parameters:
- `game_id` (uint8_t): Unique identifier for the game

#### Example:
```bash
cleos push action gamerewards removegame '[1]' -p gamerewards@active
```

### `recordgame`

Records game data for one or more players.

#### Parameters:
- `records` (vector<game_record_data>): List of game record data structures:
  - `game_id` (uint8_t): ID of the game
  - `player` (name): Account of the player
  - `stats_names` (vector<name>): Names of stats
  - `stats_values` (vector<uint64_t>): Corresponding stat values
  - `completion_time` (time_point_sec): Game completion time

#### Example:
```bash
cleos push action gamerewards recordgame '[[{
    "game_id": 1,
    "player": "alice",
    "stats_names": ["kills"],
    "stats_values": [10],
    "completion_time": "2025-01-01T12:00:00"
}]]' -p gamerewards@active
```

### `clearrecord`

Removes game records based on filters.

#### Parameters:
- `record_ids` (vector<uint64_t>): List of record IDs to remove

#### Example:
```bash
cleos push action gamerewards clearrecord '[[1, 2, 3]]' -p gamerewards@active
```

### `setrewardcfg`

Configures how rewards are distributed for a game.

#### Parameters:
- `game_id` (uint8_t): ID of the game
- `destination_contract` (name): Contract to send rewards to
- `memo_template` (string): Template for the memo field
- `use_direct_transfer` (bool): Whether to use direct token transfer

#### Example:
```bash
cleos push action gamerewards setrewardcfg '[1, "boid", "deposit boid_id={player}", true]' -p gamerewards@active
```

### `distribute`

Distributes rewards to players based on game stats.

#### Parameters:
- `game_id` (uint8_t): ID of the game
- `cycle_number` (uint32_t): Cycle number to distribute rewards for
- `stat_name` (name): Stat to use for reward calculation
- `total_reward` (asset): Total amount of reward to distribute
- `token_contract` (name): Contract of the reward token
- `reward_percentages` (vector<uint8_t>): List of percentages for each tier

#### Example:
```bash
cleos push action gamerewards distribute '[1, 1, "kills", "100.0000 TLOS", "eosio.token", [50, 30, 20]]' -p gamerewards@active
```

### `settoken`

Configures a token for reward distribution.

#### Parameters:
- `token_contract` (name): Contract that manages the token
- `token_symbol` (symbol): Token symbol with precision
- `enabled` (bool): Whether token is available for use

#### Example:
```bash
cleos push action gamerewards settoken '["eosio.token", "4,TLOS", true]' -p gamerewards@active
```

## Building the Contract
The contract name is configurable at build time. Use the build script:
```bash
./buildContract.sh <contract_name>
```

For example:
```bash
./buildContract.sh scores.boid
```

## REWARDS DISTRIBUTION
# `distribute` Action

## Description

The `distribute` action is responsible for allocating rewards to players based on their performance in a specific game and cycle. It calculates and distributes rewards to the top performers using the defined reward percentages for the game. This action ensures that rewards are distributed fairly and efficiently while updating the game records to reflect the distribution.

---

## Purpose

1. **Reward Distribution**: Allocate rewards to players based on their performance in a particular game cycle.
2. **Token Management**: Use configured tokens and validate their contracts for reward distribution.
3. **Leaderboard Management**: Determine top players based on their scores for a specific stat and allocate rewards proportionally.
4. **Cycle Validation**: Ensure that the specified cycle is valid and rewards are distributed only once per cycle.

---

## Parameters

- **`game_id` (uint8_t)**: The unique identifier for the game.
- **`cycle_number` (uint32_t)**: The cycle number for which rewards are being distributed.
- **`stat_name` (eosio::name)**: The stat used to rank players and calculate rewards.
- **`total_reward` (eosio::asset)**: The total amount of rewards to distribute.
- **`token_contract` (eosio::name)**: The smart contract managing the token used for rewards.
- **`reward_percentages` (std::vector<uint8_t>)**: A vector of reward percentages for each tier.

---

## Workflow

### Step 1: Validate Inputs
- Check if the `total_reward` is positive.
- Ensure `reward_percentages` is not empty and sums to 100.
- Validate the existence and active status of the specified game (`game_id`).
- Validate the token configuration for `total_reward.symbol` and `token_contract`.

### Step 2: Fetch Player Scores
- Retrieve all game records for the specified `game_id` and `cycle_number` from the `gamerecords_table`.
- Extract scores based on the `stat_name` and store them in a list of player-score pairs.

### Step 3: Sort and Select Top Performers
- Sort players by their scores in descending order.
- Limit the number of players to the maximum allowed reward tiers defined in the global configuration.

### Step 4: Distribute Rewards
- For each top player, calculate the reward based on their tier's percentage.
- Transfer the reward directly to the player or to a destination contract, depending on the configuration.
- Generate a memo for the transfer using a predefined template.

### Step 5: Update Game Records
- Mark all game records for the cycle as `rewards_distributed = true` to prevent duplicate distributions.

---

## Example

### Scenario:
Distribute 1000 TLOS tokens for "kills" in `game_id = 1`, `cycle_number = 2`. The reward percentages are `[50, 30, 20]` for the top 3 players.

### Command:
```bash
cleos push action gamerewards distribute '[
    1,
    2,
    "kills",
    "1000.0000 TLOS",
    "eosio.token",
    [50, 30, 20]
]' -p gamerewards@active
```