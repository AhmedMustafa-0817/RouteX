#ifndef QUEUE_H
#define QUEUE_H

#include "Shipment.h"

class Queue
{
private:

    class QueueNode
    {
    public:
        Shipment data;
        QueueNode* next;

        QueueNode(Shipment shipment)
        {
            data = shipment;
            next = nullptr;
        }
    };

    QueueNode* front;
    QueueNode* rear;

public:
    Queue();

    ~Queue();

    bool isEmpty();

    void enqueue(Shipment shipment);

    Shipment dequeue();

    Shipment* getFront();

    void display();
};

#endif