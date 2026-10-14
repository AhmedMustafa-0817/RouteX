
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
    tm *localTime = localtime(&now);

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
        status);

    if (!blockchain.addTransaction(event))
        return false;

    nextTransactionID++;
    return true;
}

bool ShipmentManager::createShipment(Shipment shipment)
{
    int id = shipment.getShipmentID();

    if (id <= 0 || registry.search(id) != nullptr)
    {
        cout << "Invalid or duplicate shipment ID.\n";
        return false;
    }

    if (!registry.insert(shipment))
        return false;

    processingQueue.enqueue(shipment);

    if (!recordEvent(
            id, "CREATED",
            shipment.getSourceCity(),
            shipment.getStatus()))
    {
        cout << "Warning: shipment created, but event recording failed.\n";
    }

    return true;
}

Shipment *ShipmentManager::searchShipment(int shipmentID)
{
    return registry.search(shipmentID);
}

bool ShipmentManager::processNextShipment()
{
    if (processingQueue.isEmpty())
    {
        cout << "No shipments waiting for processing.\n";
        return false;
    }

    Shipment shipment = processingQueue.dequeue();
    int id = shipment.getShipmentID();

    Shipment *storedShipment = registry.search(id);

    if (storedShipment == nullptr)
    {
        cout << "Shipment not found in registry.\n";
        return false;
    }

    storedShipment->setStatus("DISPATCHED");

    if (!recordEvent(
            id, "DISPATCHED",
            storedShipment->getSourceCity(),
            "DISPATCHED"))
    {
        cout << "Warning: dispatch event recording failed.\n";
    }

    cout << "Shipment " << id << " dispatched.\n";
    return true;
}

bool ShipmentManager::recordArrival(
    int shipmentID,
    string location)
{
    Shipment *shipment = registry.search(shipmentID);

    if (shipment == nullptr)
    {
        cout << "Shipment not found.\n";
        return false;
    }

    if (shipment->getStatus() != "DISPATCHED")
    {
        cout << "Only dispatched shipments can arrive.\n";
        return false;
    }

    shipment->setStatus("ARRIVED");

    if (!recordEvent(
            shipmentID, "ARRIVED",
            location, "ARRIVED"))
    {
        shipment->setStatus("DISPATCHED");
        cout << "Arrival event recording failed.\n";
        return false;
    }

    cout << "Shipment " << shipmentID << " arrived at "
         << location << ".\n";

    return true;
}

bool ShipmentManager::saveAll(const string &filename)
{
    Shipment shipments[500];
    int count = registry.getAllShipments(shipments, 500);

    ofstream file(filename);

    if (!file.is_open())
        return false;

    file << "shipmentID,sourceCity,destinationCity,"
         << "packageType,weight,status\n";

    file.close();

    for (int i = 0; i < count; i++)
    {
        if (!Storage::saveShipment(filename, shipments[i]))
            return false;
    }

    cout << "Saved " << count << " shipment(s).\n";
    return true;
}

bool ShipmentManager::loadAll(const string &filename)
{
    Shipment shipments[500];
    int count = 0;

    if (!Storage::loadShipments(filename, shipments, 500, count))
        return false;

    for (int i = 0; i < count; i++)
    {
        if (registry.search(shipments[i].getShipmentID()) != nullptr)
            continue;

        if (registry.insert(shipments[i]))
        {
            if (shipments[i].getStatus() == "CREATED" ||
                shipments[i].getStatus() == "PENDING")
            {
                processingQueue.enqueue(shipments[i]);
            }
        }
    }

    cout << "Loaded " << count << " shipment record(s).\n";
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
