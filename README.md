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
to do
## Actions and Usage
to do

## Building the Contract
The contract name is configurable at build time. Use the build script:
```bash
./buildContract.sh <contract_name>
```

For example:
```bash
./buildContract.sh scores.boid

```
