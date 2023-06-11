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

int main(int argc, char *argv[]) {
    auto *db = new eevm::ErigonDB();
    eevm::ErigonGlobalState gs(db);

    auto bytes = eevm::to_bytes("0x6982508145454ce325ddbe47a25d4ec3d2311933");
    eevm::Address pepe_contract_addr = eevm::from_big_endian(bytes.data(), bytes.size());
    auto& to = pepe_contract_addr;

    bytes = eevm::to_bytes("0x6cC5F688a315f3dC28A7781717a9A798a59fDA7b");
    eevm::Address okx_holder_addr = eevm::from_big_endian(bytes.data(), bytes.size());
    auto& from = okx_holder_addr;

    auto input = eevm::to_bytes(SIG_TOTAL_SUPPLY);

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

    auto result = exec_result.output;
    uint256_t total_supply = eevm::from_big_endian(result.data(), result.size());
    fmt::print("Total supply is {}\n", total_supply);

    return 0;
}

