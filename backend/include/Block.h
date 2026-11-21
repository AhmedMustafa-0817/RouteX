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
        EventNode* next;

        EventNode(TransferEvent newEvent);
    };

    int index;
    string timestamp;
    string previousHash;
    string currentHash;

    EventNode* eventHead;
    EventNode* eventTail;
    int eventCount;

    Block* next;

public:
    Block();
    Block(int index, string timestamp, string previousHash);
    ~Block();

    Block(const Block&) = delete;
    Block& operator=(const Block&) = delete;

    bool addEvent(TransferEvent event);
    TransferEvent* getEventAt(int position);

    void display();

    int getIndex();
    string getTimestamp();
    string getPreviousHash();
    string getCurrentHash();
    int getEventCount();

    Block* getNext();

    void setCurrentHash(string hash);
    void setPreviousHash(string hash);
    void setNext(Block* nextBlock);
};

#endif