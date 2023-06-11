// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include "eEVM/globalstate.h"
#include "eEVM/erigon/erigonaccount.h"
#include "eEVM/erigon/erigonstorage.h"
#include "eEVM/erigon/erigondb.h"

namespace eevm
{
  class ErigonGlobalState : public GlobalState
  {
  public:
    using StateEntry = std::pair<ErigonAccount, ErigonStorage>;

  private:
    ErigonDB *db = nullptr;

    Block currentBlock = {};

    std::map<Address, StateEntry> accounts;

  public:
    ErigonGlobalState(ErigonDB *_db) : db(_db) {}
    explicit ErigonGlobalState(ErigonDB *_db, Block b) : db(_db), currentBlock(std::move(b)) {}

    virtual void remove(const Address& addr) override;

    AccountState get(const Address& addr) override;
    AccountState create(
      const Address& addr, const uint256_t& balance, const Code& code) override;

    bool exists(const Address& addr);
    size_t num_accounts();

    virtual const Block& get_current_block() override;
    virtual uint256_t get_block_hash(uint8_t offset) override;

    /**
     * For tests which require some initial state, allow manual insertion of
     * pre-constructed accounts
     */
    void insert(const StateEntry& e);

    friend void to_json(nlohmann::json&, const ErigonGlobalState&);
    friend void from_json(const nlohmann::json&, ErigonGlobalState&);
    friend bool operator==(const ErigonGlobalState&, const ErigonGlobalState&);
  };

  void to_json(nlohmann::json&, const ErigonGlobalState&);
  void from_json(const nlohmann::json&, ErigonGlobalState&);
  bool operator==(const ErigonGlobalState&, const ErigonGlobalState&);
} // namespace eevm
