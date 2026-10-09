#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "Shipment.h"

class HashTable
{
private:
    class HashNode
    {
    public:
        Shipment shipment;
        HashNode *next;

        HashNode(Shipment shipment);
    };

    HashNode **buckets;
    int bucketCount;

    int hashFunction(int shipmentID);

public:
    HashTable(int size);
    ~HashTable();

    bool insert(Shipment shipment);
    Shipment *search(int shipmentID);
    bool remove(int shipmentID);
    void display();
};

#endif