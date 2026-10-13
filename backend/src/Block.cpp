#include "Block.h"
#include <iostream>

using namespace std;

Block::EventNode::EventNode(TransferEvent newEvent)
    : event(newEvent), next(NULL)
{
}

Block::Block()
{
    index = 0;
    timestamp = "";
    previousHash = "0";
    currentHash = "";

    eventHead = NULL;
    eventTail = NULL;
    eventCount = 0;

    next = NULL;
}

Block::Block(int blockIndex, string blockTimestamp,
             string blockPreviousHash)
{
    index = blockIndex;
    timestamp = blockTimestamp;
    previousHash = blockPreviousHash;
    currentHash = "";

    eventHead = NULL;
    eventTail = NULL;
    eventCount = 0;

    next = NULL;
}

Block::~Block()
{
    EventNode* current = eventHead;

    while (current != NULL)
    {
        EventNode* temporary = current;
        current = current->next;
        delete temporary;
    }

    eventHead = NULL;
    eventTail = NULL;
    eventCount = 0;
}

bool Block::addEvent(TransferEvent event)
{
    EventNode* newNode = new EventNode(event);

    if (newNode == NULL)
    {
        return false;
    }

    if (eventHead == NULL)
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

TransferEvent* Block::getEventAt(int position)
{
    if (position < 0 || position >= eventCount)
    {
        return NULL;
    }

    EventNode* current = eventHead;

    for (int i = 0; i < position; i++)
    {
        current = current->next;
    }

    return &(current->event);
}

void Block::display()
{
    cout << "\n========== BLOCK ==========\n";
    cout << "Index: " << index << endl;
    cout << "Timestamp: " << timestamp << endl;
    cout << "Previous Hash: " << previousHash << endl;
    cout << "Current Hash: " << currentHash << endl;
    cout << "Event Count: " << eventCount << endl;

    EventNode* current = eventHead;
    int number = 1;

    while (current != NULL)
    {
        cout << "\nEvent " << number << ":\n";
        current->event.display();

        current = current->next;
        number++;
    }

    cout << "===========================\n";
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

Block* Block::getNext()
{
    return next;
}

void Block::setCurrentHash(string hash)
{
    currentHash = hash;
}

void Block::setPreviousHash(string hash)
{
    previousHash = hash;
}

void Block::setNext(Block* nextBlock)
{
    next = nextBlock;
}