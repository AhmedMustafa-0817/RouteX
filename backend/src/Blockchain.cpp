#include "Blockchain.h"
#include "SHA256.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace
{
    string encodeString(const string& value)
    {
        stringstream output;
        output << value.length() << ":" << value;
        return output.str();
    }
}

Blockchain::Blockchain()
{
    head = NULL;
    tail = NULL;
    blockCount = 0;

    Block* genesis = new Block(0, "GENESIS", "0");

    genesis->setCurrentHash(calculateBlockHash(genesis));

    head = genesis;
    tail = genesis;
    blockCount = 1;
}

Blockchain::~Blockchain()
{
    Block* current = head;

    while (current != NULL)
    {
        Block* temporary = current;
        current = current->getNext();

        delete temporary;
    }

    head = NULL;
    tail = NULL;
    blockCount = 0;
}

string Blockchain::serializeBlock(Block* block)
{
    if (block == NULL)
    {
        return "";
    }

    stringstream data;

    data << block->getIndex() << "|";
    data << encodeString(block->getTimestamp()) << "|";
    data << encodeString(block->getPreviousHash()) << "|";
    data << block->getEventCount() << "|";

    for (int i = 0; i < block->getEventCount(); i++)
    {
        TransferEvent* event = block->getEventAt(i);

        if (event != NULL)
        {
            data << event->getTransactionID() << "|";
data << event->getShipmentID() << "|";
            data << encodeString(event->getEventType());
            data << encodeString(event->getLocation());
            data << encodeString(event->getTimestamp());
            data << encodeString(event->getStatus());
        }
    }

    return data.str();
}

string Blockchain::calculateBlockHash(Block* block)
{
    return SHA256::hash(serializeBlock(block));
}

bool Blockchain::addBlock(Block* block)
{
    if (block == NULL)
    {
        return false;
    }

    if (block == head || block->getNext() != NULL)
    {
        return false;
    }

    if (tail == NULL)
    {
        return false;
    }

    if (block->getIndex() != tail->getIndex() + 1)
    {
        return false;
    }


    block->setPreviousHash(tail->getCurrentHash());

  
    block->setCurrentHash(calculateBlockHash(block));

    tail->setNext(block);
    tail = block;
    blockCount++;

    return true;
}

bool Blockchain::addTransaction(TransferEvent event)
{
    if (tail == NULL)
    {
        return false;
    }

    if (!tail->addEvent(event))
    {
        return false;
    }

    tail->setCurrentHash(calculateBlockHash(tail));

    return true;
}

bool Blockchain::verifyChain()
{
    if (head == NULL || tail == NULL || blockCount <= 0)
    {
        return false;
    }

    if (head->getIndex() != 0)
    {
        return false;
    }

    if (head->getPreviousHash() != "0")
    {
        return false;
    }

    Block* current = head;
    Block* previous = NULL;
    int actualCount = 0;

    while (current != NULL)
    {
        actualCount++;


        if (current->getIndex() != actualCount - 1)
        {
            return false;
        }

        if (previous != NULL)
        {
            if (current->getPreviousHash() !=
                previous->getCurrentHash())
            {
                return false;
            }
        }

        string recalculatedHash = calculateBlockHash(current);

        if (current->getCurrentHash() != recalculatedHash)
        {
            return false;
        }

        previous = current;
        current = current->getNext();
    }

    if (actualCount != blockCount)
    {
        return false;
    }

    if (previous != tail)
    {
        return false;
    }

    if (tail->getNext() != NULL)
    {
        return false;
    }

    return true;
}

void Blockchain::display()
{
    Block* current = head;

    while (current != NULL)
    {
        current->display();
        current = current->getNext();
    }

    cout << "\nTotal Blocks: " << blockCount << endl;
    cout << "Chain Valid: "
         << (verifyChain() ? "YES" : "NO") << endl;
}

Block* Blockchain::getHead()
{
    return head;
}

Block* Blockchain::getTail()
{
    return tail;
}

int Blockchain::getBlockCount()
{
    return blockCount;
}