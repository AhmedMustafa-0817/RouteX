
#include "../include/Block.h"
#include <iostream>

using namespace std;

Block::EventNode::EventNode(TransferEvent event)
{
    this->event = event;
    next = nullptr;
}

Block::Block()
{
    index = 0;
    timestamp = "";
    previousHash = "0";
    currentHash = "PENDING";

    eventHead = nullptr;
    eventTail = nullptr;
    eventCount = 0;

    next = nullptr;
}

Block::Block(int index, string timestamp, string previousHash)
{
    this->index = index;
    this->timestamp = timestamp;
    this->previousHash = previousHash;
    currentHash = "PENDING";

    eventHead = nullptr;
    eventTail = nullptr;
    eventCount = 0;

    next = nullptr;
}

Block::~Block()
{
    EventNode *current = eventHead;

    while (current != nullptr)
    {
        EventNode *temporary = current;
        current = current->next;
        delete temporary;
    }

    eventHead = nullptr;
    eventTail = nullptr;
    eventCount = 0;
}

bool Block::addEvent(TransferEvent event)
{
    EventNode *newNode = new EventNode(event);

    if (newNode == nullptr)
    {
        return false;
    }

    if (eventHead == nullptr)
    {
        eventHead = newNode;
        eventTail = newNode;
    }
    else
    {
        eventTail->next = newNode;
        eventTail = newNode;
    }

    eventCount++;
    return true;
}

void Block::display()
{
    cout << "Block Index: " << index << endl;
    cout << "Timestamp: " << timestamp << endl;
    cout << "Previous Hash: " << previousHash << endl;
    cout << "Current Hash: " << currentHash << endl;
    cout << "Event Count: " << eventCount << endl;

    cout << "--- Events ---" << endl;

    EventNode *current = eventHead;

    while (current != nullptr)
    {
        current->event.display();
        cout << "----------------" << endl;
        current = current->next;
    }
}

int Block::getIndex()
{
    return index;
}

string Block::getTimestamp()
{
    return timestamp;
}

string Block::getPreviousHash()
{
    return previousHash;
}

string Block::getCurrentHash()
{
    return currentHash;
}

int Block::getEventCount()
{
    return eventCount;
}

Block *Block::getNext()
{
    return next;
}

void Block::setCurrentHash(string hash)
{
    currentHash = hash;
}

void Block::setNext(Block *nextBlock)
{
    next = nextBlock;
}
