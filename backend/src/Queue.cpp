#include "../include/Queue.h"
#include <iostream>

using namespace std;

Queue::Queue()
{
    front = nullptr;
    rear = nullptr;
}

Queue::~Queue()
{
    while (!isEmpty())
    {
        dequeue();
    }
}

bool Queue::isEmpty()
{
    return front == nullptr;
}

void Queue::enqueue(Shipment shipment)
{
    QueueNode* newNode = new QueueNode(shipment);

    if (isEmpty())
    {
        front = newNode;
        rear = newNode;
    }
    else
    {
        rear->next = newNode;
        rear = newNode;
    }
}

Shipment Queue::dequeue()
{
    if (isEmpty())
    {
        cout << "Queue is empty." << endl;
        return Shipment();
    }

    QueueNode* temp = front;
    Shipment removedShipment = temp->data;

    front = front->next;

    if (front == nullptr)
    {
        rear = nullptr;
    }

    delete temp;

    return removedShipment;
}

Shipment* Queue::getFront()
{
    if (isEmpty())
    {
        return nullptr;
    }

    return &front->data;
}

void Queue::display()
{
    if (isEmpty())
    {
        cout << "Queue is empty." << endl;
        return;
    }

    QueueNode* temp = front;

    while (temp != nullptr)
    {
        cout << "------------------" << endl;
        temp->data.display();
        temp = temp->next;
    }

    cout << "------------------" << endl;
}