// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "eEVM/erigon/erigonglobalstate.h"
#include "eEVM/erigon/erigondb.h"

namespace eevm
{
  void ErigonGlobalState::remove(const Address& addr)
  {
    // accounts.erase(addr);
    // removed[addr] = true;
    accounts.erase(addr);
    create(addr, 0, {});
  }

  AccountState ErigonGlobalState::get(const Address& addr)
  {
    const auto acc = accounts.find(addr);
    if (acc != accounts.cend())
      return acc->second;

    auto acc2 = db->get_account(addr);
    if(acc2 != nullptr) {
        insert({
            ErigonAccount(addr, acc2->get_balance(), acc2->get_code(), acc2->get_nonce()),
            ErigonStorage(acc2)
        });

        return get(addr);
    }

    return create(addr, 0, {});
  }

  AccountState ErigonGlobalState::create(
    const Address& addr, const uint256_t& balance, const Code& code)
  {
    insert({ErigonAccount(addr, balance, code), {}});

    return get(addr);
  }

  bool ErigonGlobalState::exists(const Address& addr)
  {
    return true;
//    return accounts.find(addr) != accounts.end();
  }

  size_t ErigonGlobalState::num_accounts()
  {
    return accounts.size();
  }

  const Block& ErigonGlobalState::get_current_block()
  {
    return currentBlock;
  }

  uint256_t ErigonGlobalState::get_block_hash(uint8_t offset)
  {
    return 0u;
  }

  void ErigonGlobalState::insert(const StateEntry& p)
  {
    const auto ib = accounts.insert(std::make_pair(p.first.get_address(), p));

    assert(ib.second);
  }

  bool operator==(const ErigonGlobalState& l, const ErigonGlobalState& r)
  {
    return false;
//    return (l.accounts == r.accounts) && (l.currentBlock == r.currentBlock);
  }

  void to_json(nlohmann::json& j, const ErigonGlobalState& s)
  {
    j["block"] = s.currentBlock;
    auto o = nlohmann::json::array();
    for (const auto& p : s.accounts)
    {
      o.push_back({to_hex_string(p.first), p.second});
    }
    j["accounts"] = o;
  }

  void from_json(const nlohmann::json& j, ErigonGlobalState& a)
  {
    if (j.find("block") != j.end())
    {
      a.currentBlock = j["block"];
    }

    for (const auto& it : j["accounts"].items())
    {
      const auto& v = it.value();
      a.accounts.insert(make_pair(to_uint256(v[0]), v[1]));
    }
  }
} // namespace eevm
