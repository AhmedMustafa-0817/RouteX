#ifndef MINHEAP_H
#define MINHEAP_H

struct HeapNode
{
    double distance;
    int cityIndex;
};

class MinHeap
{
private:
    HeapNode* heap;
    int heapSize;
    int capacity;

    bool hasHigherPriority(HeapNode a, HeapNode b) const;
    void siftUp(int index);
    void siftDown(int index);
    void grow();

public:
    MinHeap(int initialCapacity = 16);
    ~MinHeap();

    MinHeap(const MinHeap&) = delete;
    MinHeap& operator=(const MinHeap&) = delete;

    bool isEmpty() const;
    void push(double distance, int cityIndex);
    bool pop(HeapNode& result);
};

#endif