
#ifndef BLOCK_H
#define BLOCK_H

#include <string>
#include "TransferEvent.h"
using namespace std;

class Block
{
private:
    class EventNode
    {
    public:
        TransferEvent event;
        EventNode *next;

        EventNode(TransferEvent event);
    };

    int index;
    string timestamp;
    string previousHash;
    string currentHash;

    EventNode *eventHead;
    EventNode *eventTail;
    int eventCount;

    Block *next;

public:
    Block();
    Block(int index, string timestamp, string previousHash);

    ~Block();

    bool addEvent(TransferEvent event);
    void display();

    int getIndex();
    string getTimestamp();
    string getPreviousHash();
    string getCurrentHash();
    int getEventCount();
    Block *getNext();

    void setCurrentHash(string hash);
    void setNext(Block *nextBlock);
};

#endif
