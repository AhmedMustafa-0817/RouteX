// CSV persistence: special characters, large registries and reload.
#include "Storage.h"
#include "ShipmentManager.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

static int failures = 0;

static void check(bool condition, const char* message)
{
    if (condition)
    {
        cout << "PASS: " << message << endl;
    }
    else
    {
        cout << "FAIL: " << message << endl;
        ++failures;
    }
}

int main()
{
    const string file = "test_storage_tmp.csv";
    std::remove(file.c_str());

    // 1. Commas, quotes and line breaks survive a save/load round trip.
    vector<Shipment> original;
    original.push_back(Shipment(1, "Karachi", "Lahore", "Books, Pens", 2.5, "Pending"));
    original.push_back(Shipment(2, "Kar\"achi", "Lahore", "\"Quoted\"", 1, "CREATED"));
    original.push_back(Shipment(3, "Multan", "Sukkur", "Line1\nLine2", 3, "DISPATCHED"));
    original.push_back(Shipment(4, "Quetta", "Peshawar", "Plain", 0.5, "ARRIVED"));

    check(Storage::saveShipments(file, original), "saveShipments succeeds");

    vector<Shipment> loaded;
    check(Storage::loadShipments(file, loaded), "loadShipments succeeds");
    check(loaded.size() == original.size(), "every row is loaded back");

    bool same = loaded.size() == original.size();
    for (size_t i = 0; same && i < loaded.size(); ++i)
    {
        same = loaded[i].getShipmentID() == original[i].getShipmentID() &&
               loaded[i].getSourceCity() == original[i].getSourceCity() &&
               loaded[i].getDestinationCity() == original[i].getDestinationCity() &&
               loaded[i].getPackageType() == original[i].getPackageType() &&
               loaded[i].getWeight() == original[i].getWeight() &&
               loaded[i].getStatus() == original[i].getStatus();
    }
    check(same, "special characters round-trip exactly");

    // 2. No temporary file is left behind after an atomic save.
    ifstream leftover(file + ".tmp");
    check(!leftover.good(), "temporary file is removed after save");
    leftover.close();

    // 3. A registry larger than the old 500-row limit is saved and reloaded.
    {
        ShipmentManager manager;
        for (int id = 1; id <= 1200; ++id)
            manager.createShipment(Shipment(id, "Karachi", "Lahore", "Box", 1, "Pending"));

        check(manager.getShipmentCount() == 1200, "manager holds 1200 shipments");
        check(manager.saveAll(file), "saveAll writes 1200 shipments");
    }
    {
        ShipmentManager restored;
        check(restored.loadAll(file), "loadAll reads a file with more than 500 rows");
        check(restored.getShipmentCount() == 1200, "all 1200 shipments are restored");
        check(restored.searchShipment(1200) != nullptr, "the last shipment can be found after reload");
        check(restored.verifyBlockchain(), "blockchain rebuilt from the file is valid");
    }

    // 4. Malformed rows are skipped without dropping the valid ones.
    {
        ofstream out(file, ios::trunc);
        out << "shipmentID,sourceCity,destinationCity,packageType,weight,status\n"
            << "10,Karachi,Lahore,Box,1,Pending\n"
            << "not-a-number,Karachi,Lahore,Box,1,Pending\n"
            << "11,too,few,columns\n"
            << "12,Multan,Sukkur,Box,2,Pending\r\n";
    }
    loaded.clear();
    check(Storage::loadShipments(file, loaded), "file with bad rows still loads");
    check(loaded.size() == 2, "only the two valid rows are loaded");

    // 5. A missing file reports failure instead of crashing.
    std::remove(file.c_str());
    loaded.clear();
    check(!Storage::loadShipments(file, loaded), "missing file returns false");

    cout << (failures == 0 ? "\nAll storage tests passed.\n" : "\nStorage tests FAILED.\n");
    return failures == 0 ? 0 : 1;
}
