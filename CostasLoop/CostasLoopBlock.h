#ifndef __COSTAS_LOOP_BLOCK_H__
#define __COSTAS_LOOP_BLOCK_H__

#include "../Memory/Block.h"
#include "CostasLoop.h"

class CostasLoopBlock : public Block
{
public:
    CostasLoopBlock();

    CostasLoopBlock(Block * block, CostasLoop * costa, SignalType type);
    
    CostasLoopBlock& operator=(const CostasLoopBlock& src);

    void free();
    bool is_free();
    size_t get_size();
    void reset();
    void hard_reset();
    bool next();
    float ** get_iterator();
    void print();
    ~CostasLoopBlock();
    float * get_pointer(SignalType type);
    CostasLoop * costa;

private:
    SignalType type;
    Block * block;
    float in_phase_sig;
    float qu_phase_sig;
    float freq_est_sig;
    float lock_sig;
    float error_sig;
    float * in_phase_ptr;
    float * qu_phase_ptr;
    float * freq_est_ptr;
    float * lock_ptr;
    float * error_ptr;
    float * ptr;
    float ** block_iter;
};

#endif
