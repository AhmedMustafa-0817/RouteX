#ifndef SHIPMENT_H
#define SHIPMENT_H

#include <string>
using namespace std;

class Shipment
{
private:
    int shipmentID;
    string sourceCity;
    string destinationCity;
    string packageType;
    double weight;
    string status;

public:
    Shipment();

    Shipment(int id, string source, string destination,
             string type, double weight, string status);

    int getShipmentID();
    string getSourceCity();
    string getDestinationCity();
    string getPackageType();
    double getWeight();
    string getStatus();

    void setStatus(string newStatus);

    void display();
};

#endif