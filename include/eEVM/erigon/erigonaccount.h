// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include "eEVM/account.h"
#include "eEVM/erigon/erigondb.h"

#include <nlohmann/json.hpp>

namespace eevm
{
  class ErigonAccount : public Account
  {
  private:
    Address address = {};
    uint256_t balance = {};
    Code code = {};
    Nonce nonce = {};

  public:
    ErigonAccount() = default;

    ErigonAccount(const Address& a, const uint256_t& b, const Code& c) :
      address(a),
      balance(b),
      code(c),
      nonce(0)
    {}

    ErigonAccount(
      const Address& a, const uint256_t& b, const Code& c, Nonce n) :
      address(a),
      balance(b),
      code(c),
      nonce(n)
    {}

    virtual Address get_address() const override;
    void set_address(const Address& a);

    virtual uint256_t get_balance() const override;
    virtual void set_balance(const uint256_t& b) override;

    virtual Nonce get_nonce() const override;
    void set_nonce(Nonce n);
    virtual void increment_nonce() override;

    virtual Code get_code() const override;
    virtual void set_code(Code&& c) override;
    virtual bool has_code() override;

    bool operator==(const Account&) const;

    friend void to_json(nlohmann::json&, const ErigonAccount&);
    friend void from_json(const nlohmann::json&, ErigonAccount&);
  };

  void to_json(nlohmann::json&, const ErigonAccount&);
  void from_json(const nlohmann::json&, ErigonAccount&);
} // namespace eevm
