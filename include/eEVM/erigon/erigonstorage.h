// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include "eEVM/storage.h"
#include "eEVM/erigon/erigondb.h"

#include <map>
#include <nlohmann/json.hpp>

namespace eevm
{
  /**
   * Simple std::map-backed implementation of Storage
   */
  class ErigonStorage : public Storage
  {
  private:  
    std::map<uint256_t, uint256_t> s;
    ErigonDBAccount *db_acc = nullptr;

  public:
    ErigonStorage() = default;
    ErigonStorage(const nlohmann::json& j);
    ErigonStorage(ErigonDBAccount *acc) : db_acc(acc) {};

    void store(const uint256_t& key, const uint256_t& value) override;
    uint256_t load(const uint256_t& key) override;
    bool exists(const uint256_t& key);
    bool remove(const uint256_t& key) override;

    bool operator==(const ErigonStorage& that) const;

    friend void to_json(nlohmann::json&, const ErigonStorage&);
    friend void from_json(const nlohmann::json&, ErigonStorage&);
  };

  void to_json(nlohmann::json& j, const ErigonStorage& s);
  void from_json(const nlohmann::json& j, ErigonStorage& s);
} // namespace eevm
