#ifndef __QPSK_BLOCK_H__
#define __QPSK_BLOCK_H__

#include "QPSK.h"
#include "CostasLoopBlock.h"

class QPSKBlock : public CostasLoopBlock
{
public:
    QPSKBlock(Block * block, QPSK * qpsk, SignalType type = IN_PHASE_SIGNAL);
};

#endif
