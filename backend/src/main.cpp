#include "Blockchain.h"

#include <iostream>

using namespace std;

int main()
{
    Blockchain chain;

    // Create Block 1 with a shipment event.
    Block* firstBlock =
        new Block(1, "2026-10-13 10:00:00", "");

    TransferEvent firstEvent(
        101, 103, "DISPATCHED", "Karachi",
        "2026-10-13 10:00:00", "In Transit"
    );

    if (!firstBlock->addEvent(firstEvent))
    {
        delete firstBlock;
        cout << "Could not add the first event.\n";
        return 1;
    }

    if (!chain.addBlock(firstBlock))
    {
        delete firstBlock;
        cout << "Could not add Block 1.\n";
        return 1;
    }

    // Create Block 2 with another event.
    Block* secondBlock =
        new Block(2, "2026-10-13 12:00:00", "");

    TransferEvent secondEvent(
        102, 103, "ARRIVED", "Hyderabad",
        "2026-10-13 12:00:00", "Arrived at hub"
    );

    if (!secondBlock->addEvent(secondEvent))
    {
        delete secondBlock;
        cout << "Could not add the second event.\n";
        return 1;
    }

    if (!chain.addBlock(secondBlock))
    {
        delete secondBlock;
        cout << "Could not add Block 2.\n";
        return 1;
    }

    cout << "===== ORIGINAL BLOCKCHAIN =====\n";

    if (chain.verifyChain())
    {
        cout << "VALID: " << chain.getLastVerificationError() << endl;
    }
    else
    {
        cout << "INVALID: " << chain.getLastVerificationError() << endl;
        return 1;
    }

    // Simulate an unauthorized change to an older event.
    TransferEvent* oldEvent = firstBlock->getEventAt(0);

    if (oldEvent == NULL)
    {
        cout << "Could not find the event to test.\n";
        return 1;
    }

    oldEvent->setLocation("Lahore");

    cout << "\n===== AFTER TAMPERING =====\n";

    if (chain.verifyChain())
    {
        cout << "VALID: No modification detected.\n";
    }
    else
    {
        cout << "TAMPERING DETECTED: "
             << chain.getLastVerificationError() << endl;
    }

    return 0;
}