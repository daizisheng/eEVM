#include "eEVM/erigon/erigondb.h"

namespace eevm {

ErigonDBAccount* ErigonDB::get_account(const Address& a) {
    auto it = accounts.find(a);
    if(it != accounts.end()) {
        return it->second;
    }

    uint8_t addr[32];
    to_big_endian(a, addr);

    MDBX_val key, data;
    key.iov_base = addr + 12;
    key.iov_len = 20;
    int ret = mdbx_get(txn, dbi_PlainState, &key, &data);
    if(ret == MDBX_NOTFOUND) {
        accounts[a] = nullptr;
        return nullptr;
    }

    CHECK_ERR(ret);
    auto account = new ErigonDBAccount(this, a, data);
    accounts[a] = account;
    return account;
}

uint64_t read_compressed_int(const uint8_t *&p, int &len) {
    int size = *p++;
    len--;
    uint64_t ret = 0;
    while(size--) {
        ret = ret * 256 + *p++;
        len--;
    }
    return ret;
}

void ErigonDBAccount::parse_data(const MDBX_val& data) {
    auto *p = static_cast<const uint8_t*>(data.iov_base);
    int len = data.iov_len;

    //cout << to_hex_string(p, p + len) << endl;
    
    int flag = *p++;
    len--;

    if(flag & 1) {
        nonce = read_compressed_int(p, len);
    }
    if(flag & 2) {
        balance = read_compressed_int(p, len);
    }
    if(flag & 4) {
        //uint64_t x = 
        read_compressed_int(p, len); // ignore it
        //cout << "x is " << x << endl;
    }
    if(flag & 8) {
        int size = *p++;
        len--;
        if(size != len || size != 32) {
            cerr << "invalid size or len size=" << size << " len=" << len << endl;
            exit(1);
        }

        code_hash = from_big_endian(p);

        uint8_t tmp[32];
        memcpy(tmp, p, 32);

        MDBX_val key, val;
        key.iov_base = tmp;
        key.iov_len = 32;
        CHECK_ERR(mdbx_get(db->txn, db->dbi_Code, &key, &val));

        p = static_cast<const uint8_t*>(val.iov_base);
        code.resize(val.iov_len);
        code.insert(code.begin(), p, p + val.iov_len);
    }
}

const uint256_t& ErigonDBAccount::get_storage(const uint256_t& slot) {
    auto it = storage.find(slot);
    if(it != storage.end()) {
        return it->second;
    }

    MDBX_val key, val;

    uint8_t tmp[32 + 8] = {};
    to_big_endian(addr, tmp);
    tmp[39] = 1;
    key.iov_base = tmp + 12;
    key.iov_len = 20 + 8;

    uint8_t tmp2[32] = {};
    to_big_endian(slot, tmp2);
    val.iov_base = tmp2;
    val.iov_len = 32;

    MDBX_cursor *cursor;
    CHECK_ERR(mdbx_cursor_open(db->txn, db->dbi_PlainState, &cursor));
    int ret = mdbx_cursor_get(cursor, &key, &val, MDBX_GET_BOTH_RANGE);
    if(ret == MDBX_NOTFOUND) {
        return storage[slot] = 0;
    }
    CHECK_ERR(ret);

    if(val.iov_len >= 32 && key.iov_len == 28 && memcmp(key.iov_base, tmp + 12, 28) == 0 && memcmp(val.iov_base, tmp2, 32) == 0) {
        // match
        auto slot_val = from_big_endian(static_cast<const uint8_t*>(val.iov_base) + 32, val.iov_len - 32);
        return storage[slot] = slot_val;
    }

    return storage[slot] = 0;
}

}
