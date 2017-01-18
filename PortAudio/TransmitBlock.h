#ifndef __TRANSMIT_BLOCK_H__
#define __TRANSMIT_BLOCK_H__

#include "../Transmitter/Prefix.h"
#include "../Transmitter/BPSK.h"
#include "../Memory/Block.h"

class TransmitBlock
{
public:
    TransmitBlock():
        pref(NULL),
        bpsk(NULL),
        block(NULL)
    {
    }

    void init(Prefix * pref, BPSK * bpsk, Block * block)
    {
        this->pref = pref;
        this->bpsk = bpsk;
        this->block = block;
    }

    Block * process()
    {
        return bpsk->process(pref->process(block));
    }

private:
    Prefix * pref;
    BPSK * bpsk;
    Block * block;
};

#endif
