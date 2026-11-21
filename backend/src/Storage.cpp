
#include "../include/Storage.h"
#include <fstream>
#include <sstream>
#include <iostream>

bool Storage::saveShipment(const string &filename, Shipment shipment)
{
    // Check whether the file is missing or empty.
    ifstream checkFile(filename);
    bool writeHeader = !checkFile.good();

    if (checkFile.good())
    {
        checkFile.seekg(0, ios::end);
        writeHeader = (checkFile.tellg() == 0);
        checkFile.close();
    }

    ofstream file(filename, ios::app);

    if (!file.is_open())
        return false;

    if (writeHeader)
    {
        file << "shipmentID,sourceCity,destinationCity,"
             << "packageType,weight,status\n";
    }

    // Simple CSV: fields must not contain commas or newlines.
    file << shipment.getShipmentID() << ","
         << shipment.getSourceCity() << ","
         << shipment.getDestinationCity() << ","
         << shipment.getPackageType() << ","
         << shipment.getWeight() << ","
         << shipment.getStatus() << "\n";

    return file.good();
}

bool Storage::loadShipments(
    const string &filename,
    Shipment shipments[],
    int capacity,
    int &count)
{
    ifstream file(filename);

    if (!file.is_open())
        return false;

    count = 0;

    string line;

    // Skip the CSV header.
    if (!getline(file, line))
        return true;

    while (getline(file, line))
    {
        if (line.empty())
            continue;

        if (count >= capacity)
        {
            cout << "Shipment capacity reached.\n";
            return false;
        }

        stringstream ss(line);

        string idText, source, destination;
        string type, weightText, status;

        if (!getline(ss, idText, ',') ||
            !getline(ss, source, ',') ||
            !getline(ss, destination, ',') ||
            !getline(ss, type, ',') ||
            !getline(ss, weightText, ',') ||
            !getline(ss, status))
        {
            cout << "Skipping malformed CSV row.\n";
            continue;
        }

        try
        {
            size_t idPosition, weightPosition;

            int id = stoi(idText, &idPosition);
            double weight = stod(weightText, &weightPosition);

            if (idPosition != idText.size() ||
                weightPosition != weightText.size() ||
                id <= 0 || weight < 0)
            {
                cout << "Skipping invalid shipment data.\n";
                continue;
            }

            shipments[count] = Shipment(
                id, source, destination, type, weight, status);

            count++;
        }
        catch (...)
        {
            cout << "Skipping CSV row with invalid numbers.\n";
        }
    }

    return !file.bad();
}
