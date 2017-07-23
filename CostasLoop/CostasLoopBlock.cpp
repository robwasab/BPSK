#include "CostasLoopBlock.h"

CostasLoopBlock::CostasLoopBlock(Block * block,
        CostasLoop * costa, SignalType type):
    costa(costa),
    type(type),
    block(block)
{
    in_phase_sig = 0.0;
    qu_phase_sig = 0.0;
    freq_est_sig = 0.0;
    lock_sig     = 0.0;
    error_sig    = 0.0;

    in_phase_ptr = &in_phase_sig;
    qu_phase_ptr = &qu_phase_sig;
    freq_est_ptr = &freq_est_sig;
    lock_ptr     = &lock_sig;
    error_ptr    = &error_sig;

    block_iter = block->get_iterator();
    ptr = get_pointer(type);

    block->reset();
    costa->work(**block_iter, 
            in_phase_ptr,
            qu_phase_ptr,
            freq_est_ptr, 
            lock_ptr, 
            error_ptr);
}

float * CostasLoopBlock::get_pointer(SignalType type)
{
    switch(type)
    {
        case IN_PHASE_SIGNAL:
            return in_phase_ptr;

        case QU_PHASE_SIGNAL:
            return qu_phase_ptr;

        case FREQUENCY_EST_SIGNAL:
            return freq_est_ptr;

        case LOCK_SIGNAL:
            return lock_ptr;

        case ERROR_SIGNAL:
            return error_ptr;
        default:
            return ptr;
    }
}

float ** CostasLoopBlock::get_iterator()
{
    return &ptr;
}

void CostasLoopBlock::reset() {
    block->reset();
}

void CostasLoopBlock::hard_reset() {
    costa->reset();
}

bool CostasLoopBlock::next() {
    bool has_next = block->next();
    if (has_next) {
        costa->work(**block_iter, 
                in_phase_ptr,
                qu_phase_ptr,
                freq_est_ptr, 
                lock_ptr, 
                error_ptr);
        return true;
    }
    return false;
}

void CostasLoopBlock::print() {
    if (block) {
        block->print();
    }
}

void CostasLoopBlock::free() {
    block->free();
    delete this;
}

bool CostasLoopBlock::is_free() {
    return block->is_free();
}

size_t CostasLoopBlock::get_size() {
    return block->get_size();
}

CostasLoopBlock::~CostasLoopBlock() {
}
