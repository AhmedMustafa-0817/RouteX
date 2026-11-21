
#ifndef STORAGE_H
#define STORAGE_H

#include <string>
#include "Shipment.h"

using namespace std;

class Storage
{
public:
    static bool saveShipment(const string &filename, Shipment shipment);

    static bool loadShipments(
        const string &filename,
        Shipment shipments[],
        int capacity,
        int &count);
};

#endif
