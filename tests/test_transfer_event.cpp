
#include "../backend/include/TransferEvent.h"
#include <iostream>

using namespace std;

int main()
{
    TransferEvent event(
        1001,
        101,
        "Dispatched",
        "Karachi",
        "2026-10-11 10:30",
        "In Transit");

    cout << "Transaction ID: " << event.getTransactionID() << endl;
    cout << "Shipment ID: " << event.getShipmentID() << endl;
    cout << "Event Type: " << event.getEventType() << endl;
    cout << "Location: " << event.getLocation() << endl;
    cout << "Timestamp: " << event.getTimestamp() << endl;
    cout << "Status: " << event.getStatus() << endl;

    cout << "\n=== Full Event ===" << endl;
    event.display();

    return 0;
}
