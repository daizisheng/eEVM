#pragma once

#include <iostream>
#include <map>
#include <vector>

#include "mdbx.h"
#include "eEVM/account.h"

namespace eevm {
    using namespace std;

#define CHECK_ERR(x) do {\
    int rc = (x);\
    if(rc != MDBX_SUCCESS) {\
        fprintf(stderr, "file=%s line=%d %s: (%d) %s\n", __FILE__, __LINE__, #x, rc, mdbx_strerror(rc));\
        exit(-1);\
    }\
} while(0)

    class ErigonDBAccount;

    class ErigonDB {
    friend ErigonDBAccount;
    private:
        MDBX_env *env = nullptr;
        MDBX_txn *txn = nullptr;

        MDBX_dbi dbi_PlainState = -1;
        MDBX_dbi dbi_Code = -1;

        map<Address, ErigonDBAccount*> accounts;
    public:
        ErigonDB() {
            CHECK_ERR(mdbx_env_create(&env));
            CHECK_ERR(mdbx_env_set_maxdbs(env, 1000));
            CHECK_ERR(mdbx_env_set_maxreaders(env, 1000));
            CHECK_ERR(mdbx_env_open(env, "/data/ethdata/erigon/chaindata/", MDBX_RDONLY, 0));

            int dead = -1;
            CHECK_ERR(mdbx_reader_check(env, &dead));

            CHECK_ERR(mdbx_txn_begin(env, NULL, (MDBX_txn_flags_t)MDBX_RDONLY, &txn));

            CHECK_ERR(mdbx_dbi_open(txn, "Code", (MDBX_db_flags_t)0, &dbi_Code));
            CHECK_ERR(mdbx_dbi_open(txn, "PlainState", MDBX_DUPSORT, &dbi_PlainState));
        }

        ErigonDBAccount* get_account(const Address& a);
    };

    class ErigonDBAccount {
    private:
        Address addr = {};
        Code code = {};
        uint64_t balance = 0;
        uint64_t nonce = 0;
        ErigonDB *db = nullptr;

        uint256_t code_hash;

        map<uint256_t, uint256_t> storage;

        void parse_data(const MDBX_val& data);
    public:
        ErigonDBAccount(ErigonDB *_db, const Address& a, const MDBX_val& data) : addr(a), db(_db) {
            parse_data(data);
        }

        inline const Address& get_address() const {
            return addr;
        }

        inline const Code& get_code() const {
            return code;
        }

        inline uint64_t get_balance() const {
            return balance;
        }

        inline uint64_t get_nonce() const {
            return nonce;
        }

        const uint256_t& get_storage(const uint256_t& slot);
    };
}



