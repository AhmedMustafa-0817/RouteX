#include "../include/HashTable.h"
#include <iostream>

using namespace std;

HashTable::HashNode::HashNode(Shipment shipment)
{
    this->shipment = shipment;
    next = nullptr;
}

HashTable::HashTable(int size)
{
    bucketCount = size;
    buckets = new HashNode *[bucketCount];

    for (int i = 0; i < bucketCount; i++)
    {
        buckets[i] = nullptr;
    }
}

HashTable::~HashTable()
{
    for (int i = 0; i < bucketCount; i++)
    {
        HashNode *current = buckets[i];

        while (current != nullptr)
        {
            HashNode *temp = current;
            current = current->next;
            delete temp;
        }
    }

    delete[] buckets;
}

int HashTable::hashFunction(int shipmentID)
{
    return shipmentID % bucketCount;
}

bool HashTable::insert(Shipment shipment)
{
    int index = hashFunction(shipment.getShipmentID());

    HashNode *current = buckets[index];

    while (current != nullptr)
    {
        if (current->shipment.getShipmentID() == shipment.getShipmentID())
        {
            return false;
        }

        current = current->next;
    }

    HashNode *newNode = new HashNode(shipment);
    newNode->next = buckets[index];
    buckets[index] = newNode;

    return true;
}

Shipment *HashTable::search(int shipmentID)
{
    int index = hashFunction(shipmentID);

    HashNode *current = buckets[index];

    while (current != nullptr)
    {
        if (current->shipment.getShipmentID() == shipmentID)
        {
            return &current->shipment;
        }

        current = current->next;
    }

    return nullptr;
}

bool HashTable::remove(int shipmentID)
{
    int index = hashFunction(shipmentID);

    HashNode *current = buckets[index];
    HashNode *previous = nullptr;

    while (current != nullptr)
    {
        if (current->shipment.getShipmentID() == shipmentID)
        {
            if (previous == nullptr)
            {
                buckets[index] = current->next;
            }
            else
            {
                previous->next = current->next;
            }

            delete current;
            return true;
        }

        previous = current;
        current = current->next;
    }

    return false;
}

void HashTable::display()
{
    for (int i = 0; i < bucketCount; i++)
    {
        cout << "Bucket " << i << ": ";

        HashNode *current = buckets[i];

        if (current == nullptr)
        {
            cout << "Empty";
        }

        while (current != nullptr)
        {
            cout << current->shipment.getShipmentID();

            if (current->next != nullptr)
            {
                cout << " -> ";
            }

            current = current->next;
        }

        cout << endl;
    }
}