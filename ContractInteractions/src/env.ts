import fs from "fs";
import * as path from 'path';
import { z } from "zod";
import * as toml from "toml";

// Define schema with Zod
const configSchema = z.object({
  Mainnet: z.object({
    native_api: z.string().url(),
  }),
  Testnet: z.object({
    native_api: z.string().url(),
  }),
  Keys: z.object({
    priv_key: z.string(),
    testAcc_Key: z.string()
  }),
  Other: z.object({
    conctractAcc: z.string(),
  }),
});

// Load and parse the TOML configuration file
const configPath = "../config.toml";
const absolutePath = path.resolve(__dirname, configPath);
console.log('Looking for config file at:', absolutePath);
let configContent: string;

try {
  configContent = fs.readFileSync(absolutePath, "utf-8");
} catch (error: any) {
  throw new Error(`Could not read the config file: ${error.message}`);
}

let configData: any;

try {
  configData = toml.parse(configContent);
} catch (error: any) {
  throw new Error(`Error parsing TOML file: ${error.message}`);
}

// Validate and parse the configuration
const env = configSchema.safeParse(configData);
if (!env.success) {
  throw new Error("Missing or invalid configuration variables: " + JSON.stringify(env.error.formErrors, null, 2));
}

const configuration = env.data;

export default configuration;
