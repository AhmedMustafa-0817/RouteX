
#include "../backend/include/Block.h"
#include <iostream>

using namespace std;

int main()
{
    Block block(1, "2026-10-11 22:00", "0");

    Block secondBlock(2, "2026-10-11 23:00", "PENDING");

    block.setNext(&secondBlock);

    if (block.getNext() == &secondBlock)
    {
        cout << "Block linking: PASS" << endl;
    }
    else
    {
        cout << "Block linking: FAIL" << endl;
    }

    TransferEvent event1(
        1001, 101, "Created",
        "Karachi", "2026-10-11 09:00", "Pending");

    TransferEvent event2(
        1002, 101, "Dispatched",
        "Karachi", "2026-10-11 10:30", "In Transit");

    TransferEvent event3(
        1003, 101, "Delivered",
        "Lahore", "2026-10-11 18:00", "Delivered");

    block.addEvent(event1);
    block.addEvent(event2);
    block.addEvent(event3);

    cout << "Event count: " << block.getEventCount() << endl;
    cout << "First block index: " << block.getIndex() << endl;
    cout << "Previous hash: " << block.getPreviousHash() << endl;

    cout << "\n=== Block Details ===" << endl;
    block.display();

    return 0;
}
