#include "CostasLoopBlock.h"

CostasLoopBlock::CostasLoopBlock():
    type(IN_PHASE_SIGNAL),
    block(NULL),
    costa(NULL),
    in_phase_sig(0.0),
    freq_est_sig(0.0),
    lock_sig(0.0),
    error_sig(0.0),
    in_phase_ptr(&in_phase_sig),
    freq_est_ptr(&freq_est_sig),
    lock_ptr(&lock_sig),
    error_ptr(&error_sig),
    ptr(NULL),
    block_iter(NULL),
    _free(false)
{
}

CostasLoopBlock::CostasLoopBlock(Block * block,
        CostasLoop * costa, SignalType type):
    type(type),
    block(block),
    costa(costa)
{
    in_phase_sig = 0.0;
    freq_est_sig = 0.0;
    lock_sig = 0.0;
    error_sig = 0.0;

    in_phase_ptr = &in_phase_sig;
    freq_est_ptr = &freq_est_sig;
    lock_ptr = &lock_sig;
    error_ptr = &error_sig;
    block_iter = block->get_iterator();
    ptr = get_pointer(type);
    _free = false;
    ptr = get_pointer(FREQUENCY_EST_SIGNAL);

    //costa->reset();
    reset();
}

CostasLoopBlock& CostasLoopBlock::operator=(const CostasLoopBlock& src)
{
    block = src.block;
    costa = src.costa;

    in_phase_sig = 0.0;
    freq_est_sig = 0.0;
    lock_sig = 0.0;
    error_sig = 0.0;

    in_phase_ptr = &in_phase_sig;
    freq_est_ptr = &freq_est_sig;
    lock_ptr = &lock_sig;
    error_ptr = &error_sig;
    block_iter = block->get_iterator();
    ptr = get_pointer(src.type);
    _free = src._free;

    reset();

    return *this;
}

float * CostasLoopBlock::get_pointer(SignalType type)
{
    switch(type)
    {
        case IN_PHASE_SIGNAL:
            return in_phase_ptr;

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
    //costa->reset();
    in_phase_sig = 
        costa->work(**block_iter, freq_est_ptr, lock_ptr, error_ptr);
}

bool CostasLoopBlock::next() {
    if (block->next()) {
        in_phase_sig = 
            costa->work(**block_iter, freq_est_ptr, lock_ptr, error_ptr);
        return true;
    }
    else {
        return false;
    }
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
