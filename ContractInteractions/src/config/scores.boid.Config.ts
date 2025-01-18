import { createAndSendAction } from '../antelope';
import { Asset, Name, TimePointSec, UInt32, UInt8, UInt64 } from '@wharfkit/antelope';
import configuration from 'src/env';
import { ActionParams as scoresBoidActionParams } from '../types/scores.boid';

const key = configuration.Keys.priv_key;
const testAccKey = configuration.Keys.testAcc_Key;
const conctractAcc = configuration.Other.conctractAcc;

const futureDate = new Date(Date.now() + 5 * 60 * 1000); // 5 minutes from now

// 1. Initiate contract
// Set global configuration or update the existing one
async function initiateContract(chain: "mainnet" | "testnet", permission = "active") {
    try {        
      const actionName = "initcontract";
        const dataObject: scoresBoidActionParams.initcontract = {
          start_time: TimePointSec.from(futureDate),
          cycle_length_sec: UInt32.from(3600),
          max_cycle_length_sec: UInt32.from(2592000),
          max_reward_tiers: UInt8.from(10),
          min_reward_percentage: UInt8.from(1)
        };
        createAndSendAction(
            chain,
            conctractAcc,
            actionName,
            conctractAcc,
            permission,
            dataObject,
            key
        )

      console.log("Action successfully sent!");
    } catch (error) {
      const err = error as Error;
      console.error("Error during permission update flow:", err.message, err.stack);
    }
  };

// 2. Set game
// set or update game configuration
async function setGame(chain: "mainnet" | "testnet", permission = "active") {
    try {
      const actionName = "setgame";
        const dataObject: scoresBoidActionParams.setgame = {
          game_id: UInt8.from(1),
          display_name: "Boid Squadron",
          metadata: "Boid Squadron Arcade Game",
          stats_names: [Name.from("score"), Name.from("kills"), Name.from("time")]
        };
        createAndSendAction(
          chain,
          conctractAcc,
          actionName,
          conctractAcc,
          permission,
          dataObject,
          key
        )

      console.log("Action successfully sent!");
    } catch (error) {
      const err = error as Error;
      console.error("Error during permission update flow:", err.message, err.stack);
    }
  };

// remove game
  async function removeGame(chain: "mainnet" | "testnet", permission = "active") {
    try {
      const actionName = "removegame";
        const dataObject: scoresBoidActionParams.removegame = {
          game_id: UInt8.from(1)
        };
        createAndSendAction(
          chain,
          conctractAcc,
          actionName,
          conctractAcc,
          permission,
          dataObject,
          key
        )

      console.log("Action successfully sent!");
    } catch (error) {
      const err = error as Error;
      console.error("Error during permission update flow:", err.message, err.stack);
    }
  };

// 3. Set token
// settoken 
  async function setToken(chain: "mainnet" | "testnet", permission = "active") {
    try {
      const actionName = "settoken";
        const dataObject: scoresBoidActionParams.settoken = {
          token_contract: Name.from("token.boid"),
          token_symbol: Asset.Symbol.from("4,BOID"),
          enabled: true
        };
        createAndSendAction(
          chain,
          conctractAcc,
          actionName,
          conctractAcc,
          permission,
          dataObject,
          key
        )

      console.log("Action successfully sent!");
    } catch (error) {
      const err = error as Error;
      console.error("Error during permission update flow:", err.message, err.stack);
    }
  };

// 5. Record game
// recordgame - done when games are finished, can record multiple games in one transaction
async function recordGame(
  chain: "mainnet" | "testnet",
  permission: string,
  keyAcc: string,
  records: Array<{
    player: Name,
    stats_names: Name[],
    stats_values: UInt64[],
    completion_time: TimePointSec
  }>) {
  try {
    if (records.length === 0) throw new Error("No records provided");
    if (records.length > 100) throw new Error("Too many records! Maximum allowed: 100");

    const actionName = "recordgame";
    const dataObject: scoresBoidActionParams.recordgame = {
      records: records.map(record => ({
        game_id: UInt8.from(1),
        player: record.player,
        stats_names: record.stats_names,
        stats_values: record.stats_values,
        completion_time: record.completion_time
      }))
    };
    createAndSendAction(
      chain,
      conctractAcc,
      actionName,
      conctractAcc,
      permission,
      dataObject,
      keyAcc
    )

    console.log("Action successfully sent!");
  } catch (error) {
    const err = error as Error;
    console.error("Error during permission update flow:", err.message, err.stack);
  }
};

// Example usage:
// recordGame(
//   "mainnet", 
//   "active", 
//   testAccKey,
//   [
//     {
//       player: Name.from("player1"),
//       stats_names: [Name.from("score"), Name.from("kills")],
//       stats_values: [UInt64.from(100), UInt64.from(10)],
//       completion_time: TimePointSec.from(new Date())
//     },
//     {
//       player: Name.from("player2"),
//       stats_names: [Name.from("score"), Name.from("kills")],
//       stats_values: [UInt64.from(200), UInt64.from(20)],
//       completion_time: TimePointSec.from(new Date())
//     }
//   ]
// )

// clearrecord
async function clearRecord(chain: "mainnet" | "testnet", permission = "active") {
    try {
      const actionName = "clearrecord";
        const dataObject: scoresBoidActionParams.clearrecord = {
          record_ids: [UInt64.from(1), UInt64.from(2)]
        };
        createAndSendAction(
          chain,
          conctractAcc,
          actionName,
          conctractAcc,
          permission,
          dataObject,
          key
        )

      console.log("Action successfully sent!");
    } catch (error) {
      const err = error as Error;
      console.error("Error during permission update flow:", err.message, err.stack);
    }
  };

// 4. Set distribution config
async function setDistConfig(chain: "mainnet" | "testnet", permission = "active") {
    try {
      const actionName = "setdistconf";
        const dataObject: scoresBoidActionParams.setdistconf = {
          game_id: UInt8.from(1),
          destination_contract: Name.from("token.boid"),
          memo_template: "deposit boid_id={{game_id}}",
          use_direct_transfer: true
        };
        createAndSendAction(
          chain,
          conctractAcc,
          actionName,
          conctractAcc,
          permission,
          dataObject,
          key
        )

      console.log("Action successfully sent!");
    } catch (error) {
      const err = error as Error;
      console.error("Error during permission update flow:", err.message, err.stack);
    }
  };

// 6. Distribute
async function distribute(chain: "mainnet" | "testnet", permission = "active") {
    try {
      const actionName = "distribute";
        const dataObject: scoresBoidActionParams.distribute = {
          game_id: UInt8.from(1),
          cycle_number: UInt32.from(1),
          stat_name: Name.from("score"),
          total_reward: Asset.from("100.0000 BOID"),
          token_contract: Name.from("token.boid"),
          reward_percentages: [50, 30, 20]
        };
        createAndSendAction(
          chain,
          conctractAcc,
          actionName,
          conctractAcc,
          permission,
          dataObject,
          key
        )

      console.log("Action successfully sent!");
    } catch (error) {
      const err = error as Error;
      console.error("Error during permission update flow:", err.message, err.stack);
    }
  };

// initiateContract("mainnet");
// setGame("mainnet");
// setToken("mainnet");
// setDistConfig("mainnet");

// recordGame("mainnet", "active", key, [
//   {
//     player: Name.from("player1"),
//     stats_names: [Name.from("score"), Name.from("kills")],
//     stats_values: [UInt64.from(100), UInt64.from(10)],
//     completion_time: TimePointSec.from(new Date())
//   },
//   {
//     player: Name.from("player2"),
//     stats_names: [Name.from("score"), Name.from("kills")],
//     stats_values: [UInt64.from(200), UInt64.from(20)],
//     completion_time: TimePointSec.from(new Date())
//   }
// ]);


distribute("mainnet");

// removeGame("mainnet");