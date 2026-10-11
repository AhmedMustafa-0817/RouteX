#include "../include/MinHeap.h"

MinHeap::MinHeap(int initialCapacity)
{
    if (initialCapacity < 1)
        initialCapacity = 16;

    capacity = initialCapacity;
    heapSize = 0;
    heap = new HeapNode[capacity];
}

MinHeap::~MinHeap()
{
    delete[] heap;
}

bool MinHeap::hasHigherPriority(HeapNode a, HeapNode b) const
{
    if (a.distance == b.distance)
        return a.cityIndex < b.cityIndex;

    return a.distance < b.distance;
}

bool MinHeap::isEmpty() const
{
    return heapSize == 0;
}

void MinHeap::grow()
{
    capacity *= 2;

    HeapNode* biggerHeap = new HeapNode[capacity];

    for (int i = 0; i < heapSize; i++)
        biggerHeap[i] = heap[i];

    delete[] heap;
    heap = biggerHeap;
}

void MinHeap::siftUp(int index)
{
    while (index > 0)
    {
        int parent = (index - 1) / 2;

        if (!hasHigherPriority(heap[index], heap[parent]))
            break;

        HeapNode temp = heap[index];
        heap[index] = heap[parent];
        heap[parent] = temp;

        index = parent;
    }
}

void MinHeap::siftDown(int index)
{
    while (true)
    {
        int smallest = index;
        int left = 2 * index + 1;
        int right = 2 * index + 2;

        if (left < heapSize &&
            hasHigherPriority(heap[left], heap[smallest]))
        {
            smallest = left;
        }

        if (right < heapSize &&
            hasHigherPriority(heap[right], heap[smallest]))
        {
            smallest = right;
        }

        if (smallest == index)
            break;

        HeapNode temp = heap[index];
        heap[index] = heap[smallest];
        heap[smallest] = temp;

        index = smallest;
    }
}

void MinHeap::push(double distance, int cityIndex)
{
    if (heapSize == capacity)
        grow();

    HeapNode newNode;
    newNode.distance = distance;
    newNode.cityIndex = cityIndex;

    heap[heapSize] = newNode;

    siftUp(heapSize);

    heapSize++;
}

bool MinHeap::pop(HeapNode& result)
{
    if (isEmpty())
        return false;

    result = heap[0];

    heapSize--;

    if (heapSize > 0)
    {
        heap[0] = heap[heapSize];
        siftDown(0);
    }

    return true;
}