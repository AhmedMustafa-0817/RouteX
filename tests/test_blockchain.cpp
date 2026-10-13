#include "Blockchain.h"
#include <iostream>

using namespace std;

int main()
{
    cout << "Testing Blockchain...\n\n";

    Blockchain chain;

    if (chain.getHead() == NULL)
    {
        cout << "FAIL: Genesis block is missing.\n";
        return 1;
    }

    if (chain.getBlockCount() != 1)
    {
        cout << "FAIL: Initial block count is incorrect.\n";
        return 1;
    }

    if (!chain.verifyChain())
    {
        cout << "FAIL: Genesis chain verification failed.\n";
        return 1;
    }

    cout << "PASS: Genesis block created.\n";

    Block* secondBlock =
        new Block(1, "2026-10-13 10:00:00", "");

    if (!chain.addBlock(secondBlock))
    {
        delete secondBlock;
        cout << "FAIL: Could not add second block.\n";
        return 1;
    }

    if (chain.getBlockCount() != 2)
    {
        cout << "FAIL: Block count is incorrect.\n";
        return 1;
    }

    cout << "PASS: Second block added.\n";

    if (secondBlock->getPreviousHash() !=
        chain.getHead()->getCurrentHash())
    {
        cout << "FAIL: Previous hash link is incorrect.\n";
        return 1;
    }

    cout << "PASS: Previous hash link is correct.\n";


    if (!chain.verifyChain())
    {
        cout << "FAIL: Blockchain verification failed.\n";
        return 1;
    }

    cout << "PASS: Blockchain verification successful.\n";

    chain.display();

    cout << "\nAll blockchain tests passed.\n";

    return 0;
}