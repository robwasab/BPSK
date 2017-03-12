#include "QPSKBlock.h"

QPSKBlock::QPSKBlock(Block * block, QPSK * qpsk, SignalType type):
    CostasLoopBlock(block, (CostasLoop *) qpsk, type)
{

}
