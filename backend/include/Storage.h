#ifndef STORAGE_H
#define STORAGE_H

#include <string>
#include <vector>
#include "Shipment.h"

using namespace std;

class Storage
{
public:
    // Appends one shipment row (writes the header if the file is new/empty).
    static bool saveShipment(const string &filename, Shipment shipment);

    // Replaces the whole file atomically: rows are written to a temporary
    // file first, which is then renamed over the original, so a crash
    // mid-save can never leave a truncated CSV behind.
    static bool saveShipments(const string &filename, vector<Shipment> &shipments);

    // Loads every valid row. Malformed rows are skipped with a warning.
    // Returns false only if the file cannot be opened or read.
    static bool loadShipments(const string &filename, vector<Shipment> &shipments);
};

#endif
