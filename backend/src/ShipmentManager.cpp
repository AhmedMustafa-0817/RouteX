#include "../include/ShipmentManager.h"
#include "../include/TransferEvent.h"

#include <iostream>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <sstream>

ShipmentManager::ShipmentManager()
    : registry(101), nextTransactionID(1)
{
}

bool ShipmentManager::recordEvent(
    int shipmentID,
    string eventType,
    string location,
    string status)
{
    time_t now = time(nullptr);
    tm* localTime = localtime(&now);

    if (localTime == nullptr)
        return false;

    stringstream timestamp;
    timestamp << put_time(localTime, "%Y-%m-%d %H:%M:%S");

    TransferEvent event(
        nextTransactionID,
        shipmentID,
        eventType,
        location,
        timestamp.str(),
        status
    );

    if (!blockchain.addTransaction(event))
        return false;

    ++nextTransactionID;
    return true;
}

bool ShipmentManager::createShipment(Shipment shipment)
{
    int id = shipment.getShipmentID();

    if (id <= 0 || registry.search(id) != nullptr)
        return false;

    if (!registry.insert(shipment))
        return false;

    processingQueue.enqueue(shipment);

    if (!recordEvent(
            id,
            "CREATED",
            shipment.getSourceCity(),
            shipment.getStatus()))
    {
        cout << "Warning: shipment created, but event recording failed.\n";
    }

    return true;
}

Shipment* ShipmentManager::searchShipment(int shipmentID)
{
    return registry.search(shipmentID);
}

bool ShipmentManager::processNextShipment()
{
    if (processingQueue.isEmpty())
        return false;

    Shipment shipment = processingQueue.dequeue();
    int id = shipment.getShipmentID();

    Shipment* storedShipment = registry.search(id);

    if (storedShipment == nullptr)
        return false;

    storedShipment->setStatus("DISPATCHED");

    if (!recordEvent(
            id,
            "DISPATCHED",
            storedShipment->getSourceCity(),
            "DISPATCHED"))
    {
        cout << "Warning: dispatch event recording failed.\n";
    }

    return true;
}

bool ShipmentManager::recordArrival(
    int shipmentID,
    string location)
{
    Shipment* shipment = registry.search(shipmentID);

    if (shipment == nullptr ||
        shipment->getStatus() != "DISPATCHED")
        return false;

    if (!recordEvent(
            shipmentID,
            "ARRIVED",
            location,
            "ARRIVED"))
        return false;

    shipment->setStatus("ARRIVED");
    return true;
}

bool ShipmentManager::saveAll(const string& filename)
{
    Shipment shipments[500];
    int count = registry.getAllShipments(shipments, 500);

    ofstream file(filename);

    if (!file.is_open())
        return false;

    file << "shipmentID,sourceCity,destinationCity,"
         << "packageType,weight,status\n";

    file.close();

    for (int i = 0; i < count; ++i)
    {
        if (!Storage::saveShipment(filename, shipments[i]))
            return false;
    }

    return true;
}

bool ShipmentManager::loadAll(const string& filename)
{
    Shipment shipments[500];
    int count = 0;

    if (!Storage::loadShipments(filename, shipments, 500, count))
        return false;

    for (int i = 0; i < count; ++i)
    {
        int id = shipments[i].getShipmentID();

        if (registry.search(id) != nullptr)
            continue;

        if (registry.insert(shipments[i]))
        {
            string status = shipments[i].getStatus();

            if (status == "CREATED" ||
                status == "PENDING" ||
                status == "Pending")
            {
                processingQueue.enqueue(shipments[i]);
            }
        }
    }

    return true;
}

void ShipmentManager::displayShipments()
{
    registry.display();
}

void ShipmentManager::displayBlockchain()
{
    blockchain.display();
}

int ShipmentManager::getAllShipments(
    Shipment shipments[],
    int capacity)
{
    return registry.getAllShipments(shipments, capacity);
}

bool ShipmentManager::verifyBlockchain()
{
    return blockchain.verifyChain();
}

string ShipmentManager::getBlockchainVerificationError()
{
    return blockchain.getLastVerificationError();
}

int ShipmentManager::getBlockchainBlockCount()
{
    return blockchain.getBlockCount();
}