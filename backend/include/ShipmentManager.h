#ifndef SHIPMENT_MANAGER_H
#define SHIPMENT_MANAGER_H

#include "Shipment.h"
#include "HashTable.h"
#include "Queue.h"
#include "Blockchain.h"
#include "Storage.h"
#include <string>

using namespace std;

class ShipmentManager
{
private:
    HashTable registry;
    Queue processingQueue;
    Blockchain blockchain;
    int nextTransactionID;

    bool recordEvent(
        int shipmentID,
        string eventType,
        string location,
        string status
    );

public:
    ShipmentManager();

    bool createShipment(Shipment shipment);
    Shipment* searchShipment(int shipmentID);
    bool processNextShipment();
    bool recordArrival(int shipmentID, string location);

    bool saveAll(const string& filename);
    bool loadAll(const string& filename);

    void displayShipments();
    void displayBlockchain();

    int getAllShipments(Shipment shipments[], int capacity);
    bool verifyBlockchain();
    string getBlockchainVerificationError();
    int getBlockchainBlockCount();
};

#endif