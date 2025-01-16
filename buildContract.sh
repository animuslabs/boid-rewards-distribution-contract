#!/bin/bash

# Check if a contract name was provided
if [ -z "$1" ]; then
    echo "Error: Contract name is required."
    echo "Usage: $0 contract_name"
    exit 1
fi

CONTRACT_NAME=$1

# Validate contract name (must be <= 12 chars, only a-z, 1-5, .)
if [[ ! $CONTRACT_NAME =~ ^[a-z1-5.]{1,12}$ ]]; then
    echo "Error: Contract name must be 12 characters or less and only contain a-z, 1-5, or ."
    echo "Usage: $0 contract_name"
    exit 1
fi

echo ">>> Building contract: $CONTRACT_NAME"

# Create build directory if it doesn't exist
if [ ! -d "$PWD/build" ]; then
    mkdir -p build
fi

# Compile the contract with eosio-cpp
cdt-cpp \
  -I="./include/" \
  -I="./external/eosio.contracts/eosio.token/include/" \
  -DCONTRACT_NAME="\"$CONTRACT_NAME\"" \
  -o="./build/$CONTRACT_NAME.wasm" \
  -contract=$CONTRACT_NAME \
  -abigen -abigen_output="./build/$CONTRACT_NAME.abi" \
  ./src/gamesRewards.cpp

# Check for compilation success
if [ $? -eq 0 ]; then
    echo ">>> Build successful! Output files:"
    echo "    - ./build/$CONTRACT_NAME.wasm"
    echo "    - ./build/$CONTRACT_NAME.abi"
else
    echo ">>> Build failed!"
    exit 1
fi
