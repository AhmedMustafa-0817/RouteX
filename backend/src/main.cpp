
#include "../include/ShipmentManager.h"
#include <iostream>

using namespace std;

int main()
{
    ShipmentManager manager;

    cout << "===== ROUTEX SHIPMENT LIFECYCLE =====\n\n";

    // 1. Create shipments.
    Shipment s1(
        201, "Karachi", "Hyderabad",
        "Electronics", 2.5, "CREATED");

    Shipment s2(
        202, "Lahore", "Islamabad",
        "Documents", 0.5, "CREATED");

    manager.createShipment(s1);
    manager.createShipment(s2);

    // 2. Search through the custom hash table.
    cout << "\n===== SEARCH SHIPMENT =====\n";

    Shipment *found = manager.searchShipment(201);

    if (found != nullptr)
        found->display();
    else
        cout << "Shipment not found.\n";

    // 3. Process the first shipment in FIFO order.
    cout << "\n===== PROCESS SHIPMENT =====\n";
    manager.processNextShipment();

    // 4. Record arrival and update shipment status.
    cout << "\n===== RECORD ARRIVAL =====\n";
    manager.recordArrival(201, "Hyderabad");

    // 5. Display the blockchain and verify its integrity.
    cout << "\n===== BLOCKCHAIN =====\n";
    manager.displayBlockchain();

    // 6. Save current shipment records.
    cout << "\n===== SAVE SHIPMENTS =====\n";

    if (!manager.saveAll("data/demo_shipments.csv"))
    {
        cout << "Failed to save shipments.\n";
        return 1;
    }

    // 7. Reload shipment records into a fresh manager.
    cout << "\n===== RELOAD SHIPMENTS =====\n";

    ShipmentManager restoredManager;

    if (!restoredManager.loadAll("data/demo_shipments.csv"))
    {
        cout << "Failed to reload shipments.\n";
        return 1;
    }

    restoredManager.displayShipments();

    cout << "\n===== DEMO COMPLETE =====\n";

    return 0;
}
