#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include <string>
#include "Block.h"

using namespace std;

class Blockchain
{
private:
    Block* head;
    Block* tail;
    int blockCount;

    string lastVerificationError;

    string serializeBlock(Block* block);
    string calculateBlockHash(Block* block);

public:
    Blockchain();
    ~Blockchain();

    Blockchain(const Blockchain&) = delete;
    Blockchain& operator=(const Blockchain&) = delete;

    bool addBlock(Block* block);
    bool addTransaction(TransferEvent event);
    bool verifyChain();
    string getLastVerificationError();

    void display();

    Block* getHead();
    Block* getTail();
    int getBlockCount();
};

#endif