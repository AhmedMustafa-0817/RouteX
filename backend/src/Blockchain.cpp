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
    lastVerificationError = "Chain has not been verified yet.";

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
    if (block == NULL || head == NULL || tail == NULL)
    {
        return false;
    }

    // Do not add an existing block or a block already linked to others.
    if (block == head || block == tail || block->getNext() != NULL)
    {
        return false;
    }

    if (block->getIndex() != tail->getIndex() + 1)
    {
        return false;
    }

    // Link the new block to the current last block.
    block->setPreviousHash(tail->getCurrentHash());

    // Calculate its hash after setting previousHash.
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

    // Adding an event changes this block's contents.
    tail->setCurrentHash(calculateBlockHash(tail));

    return true;
}

bool Blockchain::verifyChain()
{
    lastVerificationError = "";

    if (head == NULL)
    {
        lastVerificationError = "Chain has no genesis block.";
        return false;
    }

    if (tail == NULL)
    {
        lastVerificationError = "Chain has no tail block.";
        return false;
    }

    if (blockCount <= 0)
    {
        lastVerificationError = "Stored block count is invalid.";
        return false;
    }

    if (head->getIndex() != 0)
    {
        lastVerificationError =
            "Genesis block must have index 0.";
        return false;
    }

    if (head->getPreviousHash() != "0")
    {
        lastVerificationError =
            "Genesis block previousHash must be 0.";
        return false;
    }

    Block* current = head;
    Block* previous = NULL;
    int actualCount = 0;

    while (current != NULL)
    {
        // Stops verification if the list cycles or its count is wrong.
        if (actualCount >= blockCount)
        {
            lastVerificationError =
                "Cycle detected or stored block count is too small.";
            return false;
        }

        if (current->getIndex() != actualCount)
        {
            lastVerificationError =
                "Block " + to_string(current->getIndex())
                + ": incorrect block index.";
            return false;
        }

        // Check that this block links to the preceding block.
        if (previous != NULL)
        {
            if (current->getPreviousHash() !=
                previous->getCurrentHash())
            {
                lastVerificationError =
                    "Block " + to_string(current->getIndex())
                    + ": previous hash mismatch.";
                return false;
            }
        }

        // Recalculate the hash from the block's current contents.
        string recalculatedHash = calculateBlockHash(current);

        if (current->getCurrentHash() != recalculatedHash)
        {
            lastVerificationError =
                "Block " + to_string(current->getIndex())
                + ": current hash mismatch. Data may have been changed.";
            return false;
        }

        actualCount++;
        previous = current;
        current = current->getNext();
    }

    if (actualCount != blockCount)
    {
        lastVerificationError =
            "Stored block count does not match linked-list length.";
        return false;
    }

    if (previous != tail)
    {
        lastVerificationError =
            "Tail pointer does not point to the final block.";
        return false;
    }

    if (tail->getNext() != NULL)
    {
        lastVerificationError =
            "Tail block must not point to another block.";
        return false;
    }

    lastVerificationError = "Chain verified successfully.";
    return true;
}

string Blockchain::getLastVerificationError()
{
    return lastVerificationError;
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

    bool valid = verifyChain();

    cout << "Chain Valid: " << (valid ? "YES" : "NO") << endl;
    cout << "Verification Result: "
         << getLastVerificationError() << endl;
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