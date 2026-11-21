#include "../include/Shipment.h"
#include <iostream>
Shipment::Shipment()
{
    shipmentID = 0;
    sourceCity = "";
    destinationCity = "";
    packageType = "";
    weight = 0;
    status = "Pending";
}

Shipment::Shipment(int id, string source, string destination,
                   string type, double w, string s)
{
    shipmentID = id;
    sourceCity = source;
    destinationCity = destination;
    packageType = type;
    weight = w;
    status = s;
}

int Shipment::getShipmentID()
{
    return shipmentID;
}

string Shipment::getSourceCity()
{
    return sourceCity;
}

string Shipment::getDestinationCity()
{
    return destinationCity;
}

string Shipment::getPackageType()
{
    return packageType;
}

double Shipment::getWeight()
{
    return weight;
}

string Shipment::getStatus()
{
    return status;
}

void Shipment::setStatus(string newStatus)
{
    status = newStatus;
}

void Shipment::display()
{
    cout << "Shipment ID: " << shipmentID << endl;
    cout << "From: " << sourceCity << endl;
    cout << "To: " << destinationCity << endl;
    cout << "Package Type: " << packageType << endl;
    cout << "Weight: " << weight << " kg" << endl;
    cout << "Status: " << status << endl;
}