#include "../include/ShipmentManager.h"
#include "../include/TransferEvent.h"

#include <iostream>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <sstream>

ShipmentManager::ShipmentManager()
    : registry(101), nextTransactionID(1), tampered(false),
      tamperedBlockIndex(-1), tamperedEventIndex(-1), originalLocation(""), originalHash("")
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

    if (blockchain.getTail() != nullptr &&
        (blockchain.getTail()->getIndex() == 0 || blockchain.getTail()->getEventCount() >= 2))
    {
        Block* newBlock = new Block(
            blockchain.getBlockCount(),
            timestamp.str(),
            ""
        );
        blockchain.addBlock(newBlock);
    }

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

            recordEvent(
                id,
                "CREATED",
                shipments[i].getSourceCity(),
                status
            );
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

Block* ShipmentManager::getBlockchainHead()
{
    return blockchain.getHead();
}

bool ShipmentManager::tamperFirstEvent()
{
    if (tampered)
        return true;

    Block* curr = blockchain.getHead();
    while (curr != nullptr)
    {
        if (curr->getEventCount() > 0)
        {
            TransferEvent* ev = curr->getEventAt(0);
            if (ev != nullptr)
            {
                originalLocation = ev->getLocation();
                tamperedBlockIndex = curr->getIndex();
                tamperedEventIndex = 0;
                ev->setLocation("UNAUTHORIZED_LOCATION");
                tampered = true;
                return true;
            }
        }
        curr = curr->getNext();
    }

    if (blockchain.getTail() != nullptr)
    {
        originalHash = blockchain.getTail()->getCurrentHash();
        tamperedBlockIndex = blockchain.getTail()->getIndex();
        blockchain.getTail()->setCurrentHash("tampered_corrupted_hash_value");
        tampered = true;
        return true;
    }

    return false;
}

bool ShipmentManager::restoreTamper()
{
    if (!tampered)
        return true;

    if (!originalLocation.empty() && tamperedBlockIndex >= 0)
    {
        Block* curr = blockchain.getHead();
        while (curr != nullptr)
        {
            if (curr->getIndex() == tamperedBlockIndex)
            {
                TransferEvent* ev = curr->getEventAt(tamperedEventIndex);
                if (ev != nullptr)
                {
                    ev->setLocation(originalLocation);
                }
                break;
            }
            curr = curr->getNext();
        }
    }
    else if (!originalHash.empty() && tamperedBlockIndex >= 0)
    {
        Block* curr = blockchain.getHead();
        while (curr != nullptr)
        {
            if (curr->getIndex() == tamperedBlockIndex)
            {
                curr->setCurrentHash(originalHash);
                break;
            }
            curr = curr->getNext();
        }
    }

    tampered = false;
    originalLocation = "";
    originalHash = "";
    tamperedBlockIndex = -1;
    tamperedEventIndex = -1;
    return true;
}

bool ShipmentManager::isTampered() const
{
    return tampered;
}