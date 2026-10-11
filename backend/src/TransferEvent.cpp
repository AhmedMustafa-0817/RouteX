
#include "../include/TransferEvent.h"
#include <iostream>

TransferEvent::TransferEvent()
{
    transactionID = 0;
    shipmentID = 0;
    eventType = "";
    location = "";
    timestamp = "";
    status = "Pending";
}

TransferEvent::TransferEvent(int transactionID, int shipmentID,
                             string eventType, string location,
                             string timestamp, string status)
{
    this->transactionID = transactionID;
    this->shipmentID = shipmentID;
    this->eventType = eventType;
    this->location = location;
    this->timestamp = timestamp;
    this->status = status;
}

int TransferEvent::getTransactionID()
{
    return transactionID;
}

int TransferEvent::getShipmentID()
{
    return shipmentID;
}

string TransferEvent::getEventType()
{
    return eventType;
}

string TransferEvent::getLocation()
{
    return location;
}

string TransferEvent::getTimestamp()
{
    return timestamp;
}

string TransferEvent::getStatus()
{
    return status;
}

void TransferEvent::display()
{
    cout << "Transaction ID: " << transactionID << endl;
    cout << "Shipment ID: " << shipmentID << endl;
    cout << "Event Type: " << eventType << endl;
    cout << "Location: " << location << endl;
    cout << "Timestamp: " << timestamp << endl;
    cout << "Status: " << status << endl;
}
