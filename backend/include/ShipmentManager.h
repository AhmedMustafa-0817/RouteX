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
    bool tampered;
    int tamperedBlockIndex;
    int tamperedEventIndex;
    string originalLocation;
    string originalHash;

public:
    ShipmentManager();

    bool recordEvent(
        int shipmentID,
        string eventType,
        string location,
        string status
    );

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

    Block* getBlockchainHead();
    bool tamperFirstEvent();
    bool restoreTamper();
    bool isTampered() const;
};

#endif