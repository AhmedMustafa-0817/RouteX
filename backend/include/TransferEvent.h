
#ifndef TRANSFEREVENT_H
#define TRANSFEREVENT_H

#include <string>
using namespace std;

class TransferEvent
{
private:
    int transactionID;
    int shipmentID;
    string eventType;
    string location;
    string timestamp;
    string status;

public:
    TransferEvent();

    TransferEvent(int transactionID, int shipmentID,
                  string eventType, string location,
                  string timestamp, string status);

    int getTransactionID();
    int getShipmentID();
    string getEventType();
    string getLocation();
    string getTimestamp();
    string getStatus();

    void setLocation(string newLocation);
    void setTimestamp(string newTimestamp);
    void setStatus(string newStatus);

    void display();
};

#endif
