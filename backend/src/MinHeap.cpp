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

void MinHeap::swapNodes(int first, int second)
{
    HeapNode temp = heap[first];
    heap[first] = heap[second];
    heap[second] = temp;
}

// Recursive sift-up: move the node at index up until its parent
// has higher priority (or it becomes the root).
void MinHeap::siftUp(int index)
{
    // Base case: the root has no parent.
    if (index == 0)
        return;

    int parent = (index - 1) / 2;

    // Base case: heap order already holds.
    if (!hasHigherPriority(heap[index], heap[parent]))
        return;

    swapNodes(index, parent);

    // Recursive step: continue from the parent's position.
    siftUp(parent);
}

// Recursive sift-down: move the node at index down until both
// children have lower priority (or it becomes a leaf).
void MinHeap::siftDown(int index)
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

    // Base case: neither child has higher priority.
    if (smallest == index)
        return;

    swapNodes(index, smallest);

    // Recursive step: continue from the child's position.
    siftDown(smallest);
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