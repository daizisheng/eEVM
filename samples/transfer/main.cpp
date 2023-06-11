#include <iostream>
//#include "mdbx.h"
#include "eEVM/erigon/erigondb.h"
#include "eEVM/erigon/erigonaccount.h"
#include "eEVM/erigon/erigonstorage.h"
#include "eEVM/erigon/erigonglobalstate.h"
#include "eEVM/opcode.h"
#include "eEVM/processor.h"

using namespace std;

/*
    "allowance(address,address)" : "dd62ed3e",
    "approve(address,uint256)" : "095ea7b3",
    "balanceOf(address)" : "70a08231",
    "totalSupply()" : "18160ddd",
    "transfer(address,uint256)" : "a9059cbb",
    "transferFrom(address,address,uint256)" : "23b872dd"
*/

#define SIG_BALANCEOF "70a08231"
#define SIG_TOTAL_SUPPLY "18160ddd"
#define SIG_TRANSFER "a9059cbb"

eevm::Code run(eevm::ErigonGlobalState& gs, eevm::Address from, eevm::Address to, eevm::Code& input) {
    eevm::NullLogHandler ignore;
    eevm::Transaction tx(from, ignore);

    // Record a trace to aid debugging
    eevm::Trace tr;
    eevm::Processor p(gs);

    // Run the transaction
    const auto exec_result = p.run(tx, from, gs.get(to), input, 0u, &tr);

    if (exec_result.er != eevm::ExitReason::returned)
    {
        std::cerr << fmt::format("Trace:\n{}", tr) << std::endl;
        if (exec_result.er == eevm::ExitReason::threw)
        {
            throw std::runtime_error(
            fmt::format("Execution threw an error: {}", exec_result.exmsg));
        }

        throw std::runtime_error("Deployment did not return");
    }

    return exec_result.output;
}

uint256_t balance_of(eevm::ErigonGlobalState& gs, eevm::Address from, eevm::Address to, eevm::Address addr) {
    auto input = eevm::to_bytes(SIG_BALANCEOF);
    input.resize(input.size() + 32);
    eevm::to_big_endian(addr, input.data() + 4);
    auto result = run(gs, from, to, input);
    uint256_t balance = eevm::from_big_endian(result.data(), result.size());
    //fmt::print("Balance of {} is {}\n", eevm::to_hex_string(addr), balance);
    return balance;
}

int main(int argc, char *argv[]) {
    auto *db = new eevm::ErigonDB();

    auto bytes = eevm::to_bytes("0x6982508145454ce325ddbe47a25d4ec3d2311933");
    eevm::Address pepe_contract_addr = eevm::from_big_endian(bytes.data(), bytes.size());
    auto& to = pepe_contract_addr;

    bytes = eevm::to_bytes("0x6cC5F688a315f3dC28A7781717a9A798a59fDA7b");
    eevm::Address okx_holder_addr = eevm::from_big_endian(bytes.data(), bytes.size());
    auto& from = okx_holder_addr;

    bytes = eevm::to_bytes("0xF977814e90dA44bFA03b6295A0616a897441aceC");
    eevm::Address bn_holder_addr = eevm::from_big_endian(bytes.data(), bytes.size());
    auto& transfer_to = bn_holder_addr;

    eevm::ErigonGlobalState gs_0(db);
    auto input = eevm::to_bytes(SIG_TOTAL_SUPPLY);
    auto result = run(gs_0, from, to, input);
    uint256_t total_supply = eevm::from_big_endian(result.data(), result.size());
    fmt::print("Total supply is {}\n", total_supply);

    uint256_t balance_of_okx = balance_of(gs_0, from, to, okx_holder_addr);
    uint256_t balance_of_bn = balance_of(gs_0, from, to, bn_holder_addr);

    for(int amount = 1;amount < 10000;amount++) {
        if(amount % 100 == 0) {
            cerr << amount << " / " << 10000 << endl;
        }
        eevm::ErigonGlobalState gs(db);

        auto input = eevm::to_bytes(SIG_TRANSFER);
        input.resize(input.size() + 32 + 32);
        eevm::to_big_endian(transfer_to, input.data()+4);
        uint256_t transfer_amount = amount;
        eevm::to_big_endian(transfer_amount, input.data()+4+32);
        auto result = run(gs, from, to, input);

        uint256_t balance_of_okx_after = balance_of(gs, from, to, okx_holder_addr);
        uint256_t balance_of_bn_after = balance_of(gs, from, to, bn_holder_addr);

        // fmt::print("ok decrease {}\n", balance_of_okx - balance_of_okx_after);
        // fmt::print("bn increase {}\n", balance_of_bn_after - balance_of_bn);
        auto dec = balance_of_okx - balance_of_okx_after;
        auto inc = balance_of_bn_after - balance_of_bn;
        if(dec != inc || inc != amount) {
            cerr << "invalid" << endl;
            return -1;
        }
    }

    return 0;
}

