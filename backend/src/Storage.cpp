// windows.h must come before any header that does `using namespace std`,
// otherwise its `byte` typedef clashes with std::byte.
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include "../include/Storage.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>

namespace
{
    const char* CSV_HEADER =
        "shipmentID,sourceCity,destinationCity,packageType,weight,status\n";

    // Quote a field if it contains a comma, quote or line break (RFC 4180).
    string csvField(const string& value)
    {
        if (value.find_first_of(",\"\r\n") == string::npos)
            return value;

        string quoted = "\"";

        for (char c : value)
        {
            if (c == '"')
                quoted += "\"\"";
            else
                quoted += c;
        }

        quoted += "\"";
        return quoted;
    }

    void writeRow(ostream& out, Shipment& shipment)
    {
        out << shipment.getShipmentID() << ","
            << csvField(shipment.getSourceCity()) << ","
            << csvField(shipment.getDestinationCity()) << ","
            << csvField(shipment.getPackageType()) << ","
            << shipment.getWeight() << ","
            << csvField(shipment.getStatus()) << "\n";
    }

    // Split one CSV record into fields. Returns false while a quoted field
    // is still open, so the caller can append the next physical line.
    bool splitCsvRecord(const string& record, vector<string>& fields)
    {
        fields.clear();
        string current;
        bool inQuotes = false;
        bool fieldWasQuoted = false;

        for (size_t i = 0; i < record.size(); ++i)
        {
            char c = record[i];

            if (inQuotes)
            {
                if (c == '"')
                {
                    if (i + 1 < record.size() && record[i + 1] == '"')
                    {
                        current += '"';
                        ++i;
                    }
                    else
                    {
                        inQuotes = false;
                    }
                }
                else
                {
                    current += c;
                }
            }
            else if (c == '"' && current.empty() && !fieldWasQuoted)
            {
                inQuotes = true;
                fieldWasQuoted = true;
            }
            else if (c == ',')
            {
                fields.push_back(current);
                current.clear();
                fieldWasQuoted = false;
            }
            else
            {
                current += c;
            }
        }

        if (inQuotes)
            return false;

        fields.push_back(current);
        return true;
    }

    bool replaceFile(const string& source, const string& destination)
    {
#ifdef _WIN32
        return MoveFileExA(
                   source.c_str(),
                   destination.c_str(),
                   MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
        return std::rename(source.c_str(), destination.c_str()) == 0;
#endif
    }
}

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
        file << CSV_HEADER;

    writeRow(file, shipment);
    return file.good();
}

bool Storage::saveShipments(const string &filename, vector<Shipment> &shipments)
{
    string tempFile = filename + ".tmp";

    {
        ofstream file(tempFile, ios::trunc);

        if (!file.is_open())
            return false;

        file << CSV_HEADER;

        for (Shipment& shipment : shipments)
            writeRow(file, shipment);

        file.flush();

        if (!file.good())
        {
            file.close();
            std::remove(tempFile.c_str());
            return false;
        }
    }

    if (!replaceFile(tempFile, filename))
    {
        std::remove(tempFile.c_str());
        return false;
    }

    return true;
}

bool Storage::loadShipments(const string &filename, vector<Shipment> &shipments)
{
    ifstream file(filename);

    if (!file.is_open())
        return false;

    shipments.clear();

    string line;
    int lineNumber = 1;

    // Skip the CSV header.
    if (!getline(file, line))
        return true;

    while (getline(file, line))
    {
        ++lineNumber;
        int recordStart = lineNumber;

        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (line.empty())
            continue;

        // A quoted field may span several physical lines.
        string record = line;
        vector<string> fields;
        string more;

        while (!splitCsvRecord(record, fields) && getline(file, more))
        {
            ++lineNumber;
            if (!more.empty() && more.back() == '\r')
                more.pop_back();
            record += "\n" + more;
        }

        if (fields.size() != 6)
        {
            cout << "Skipping malformed CSV row at line " << recordStart << ".\n";
            continue;
        }

        try
        {
            size_t idPosition, weightPosition;

            int id = stoi(fields[0], &idPosition);
            double weight = stod(fields[4], &weightPosition);

            if (idPosition != fields[0].size() ||
                weightPosition != fields[4].size() ||
                id <= 0 || !(weight >= 0))
            {
                cout << "Skipping invalid shipment data at line " << recordStart << ".\n";
                continue;
            }

            shipments.push_back(Shipment(
                id, fields[1], fields[2], fields[3], weight, fields[5]));
        }
        catch (...)
        {
            cout << "Skipping CSV row with invalid numbers at line " << recordStart << ".\n";
        }
    }

    return !file.bad();
}
