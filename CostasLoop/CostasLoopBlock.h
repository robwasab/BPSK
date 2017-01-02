#ifndef __COSTAS_LOOP_BLOCK_H__
#define __COSTAS_LOOP_BLOCK_H__

#include "../Memory/Block.h"
#include "CostasLoop.h"

class CostasLoopBlock : public Block
{
public:
    CostasLoopBlock(Block * block, CostasLoop * costa, SignalType type);

    void free();
    bool is_free();
    size_t get_size();
    void reset();
    bool next();
    float ** get_iterator();
    void print();
    ~CostasLoopBlock();

private:
    Block * block;
    CostasLoop * costa;
    float in_phase_sig;
    float freq_est_sig;
    float lock_sig;
    float error_sig;
    float * in_phase_ptr;
    float * freq_est_ptr;
    float * lock_ptr;
    float * error_ptr;
    float * ptr;
    float ** block_iter;
    float * get_pointer(SignalType type);
};

#endif
