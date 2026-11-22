#include "Queue.h"

#include <iostream>

using namespace std;

static int failures = 0;

static void check(bool condition, const char* message)
{
    if (condition)
    {
        cout << "PASS: " << message << endl;
    }
    else
    {
        cout << "FAIL: " << message << endl;
        ++failures;
    }
}

static Shipment makeShipment(int id)
{
    return Shipment(id, "Karachi", "Lahore", "Parcel", 1.0, "Pending");
}

int main()
{
    cout << "Testing Queue...\n\n";

    Queue queue;
    check(queue.isEmpty(), "new queue is empty");
    check(queue.getFront() == nullptr, "front of empty queue is null");

    queue.enqueue(makeShipment(101));
    check(!queue.isEmpty(), "queue with one element is not empty");
    check(queue.getFront()->getShipmentID() == 101, "front is the only element");

    for (int id = 102; id <= 105; ++id)
        queue.enqueue(makeShipment(id));

    bool fifo = true;
    for (int id = 101; id <= 105; ++id)
        fifo = fifo && queue.dequeue().getShipmentID() == id;
    check(fifo, "elements leave in FIFO order");
    check(queue.isEmpty(), "queue is empty after draining");

    Shipment fromEmpty = queue.dequeue();
    check(fromEmpty.getShipmentID() == 0, "dequeue from empty queue returns a default shipment");

    // The rear pointer must be reset when the queue empties.
    queue.enqueue(makeShipment(201));
    queue.enqueue(makeShipment(202));
    check(queue.dequeue().getShipmentID() == 201 &&
          queue.dequeue().getShipmentID() == 202,
          "queue works again after being emptied");

    cout << (failures == 0 ? "\nAll queue tests passed.\n" : "\nQueue tests FAILED.\n");
    return failures == 0 ? 0 : 1;
}
