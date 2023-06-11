// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "eEVM/erigon/erigonstorage.h"

#include "eEVM/util.h"

#include <ostream>

namespace eevm
{
  ErigonStorage::ErigonStorage(const nlohmann::json& j)
  {
    for (auto it = j.cbegin(); it != j.cend(); it++)
      s.emplace(
        std::piecewise_construct,
        /* key */ std::forward_as_tuple(to_uint256(it.key())),
        /* value */ std::forward_as_tuple(to_uint256(it.value())));
  }

  void ErigonStorage::store(const uint256_t& key, const uint256_t& value)
  {
    s[key] = value;
  }

  uint256_t ErigonStorage::load(const uint256_t& key)
  {
    auto e = s.find(key);
    if (e == s.end()) {

        if(db_acc != nullptr) {
            auto v = db_acc->get_storage(key);
            if(v == 0) return 0;
            s[key] = v;
            return v;
        }
      return 0;
    }
    return e->second;
  }

  bool ErigonStorage::exists(const uint256_t& key)
  {
    // always true
    return true;
  }

  bool ErigonStorage::remove(const uint256_t& key)
  {
    store(key, 0);
    return true;
  }

  bool ErigonStorage::operator==(const ErigonStorage& that) const
  {
    return false;
  }

  void to_json(nlohmann::json& j, const ErigonStorage& s)
  {
    j = nlohmann::json::object();

    for (const auto& p : s.s)
    {
      j[to_hex_string(p.first)] = to_hex_string(p.second);
    }
  }

  void from_json(const nlohmann::json& j, ErigonStorage& s)
  {
    for (decltype(auto) it = j.cbegin(); it != j.cend(); it++)
    {
      s.s.emplace(to_uint256(it.key()), to_uint256(it.value()));
    }
  }

  inline std::ostream& operator<<(std::ostream& os, const ErigonStorage& s)
  {
    os << nlohmann::json(s).dump(2);
    return os;
  }
} // namespace eevm
