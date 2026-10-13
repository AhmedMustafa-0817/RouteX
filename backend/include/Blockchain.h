
#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include "Block.h"

class Blockchain
{
private:
    Block *head;
    Block *tail;
    int blockCount;

public:
    Blockchain();
    ~Blockchain();

    void addBlock(Block *block);
    Block *getHead();
    Block *getTail();
    int getBlockCount();
};

#endif
