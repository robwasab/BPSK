#ifndef __ATTENUATOR_H__
#define __ATTENUATOR_H__

#include "../Module/Module.h"
#include "../Memory/Block.h"


class AttenuatorBlock : public Block
{
public:
    AttenuatorBlock(Block * block, float atten) :
        block(block),
        atten(atten)
    {
        block->reset();
        iter = block->get_iterator();
        value = **iter * atten;
        ptr = &value;
    }

    void free() {
        block->free();
        delete this;
    }
    bool is_free() {
        return block->is_free();
    }
    size_t get_size() {
        return block->get_size();
    }
    void reset() {
        block->reset();
    }
    bool next() {
        bool has_next = block->next();
        if (has_next) {
            value = **iter * atten;
            return true;
        }
        return false;
    }
    float ** get_iterator() {
        return &ptr;
    }
    void print() {
        block->print();
    }
private:
    Block * block;
    float atten;
    float ** iter;
    float value;
    float * ptr;
};

class Attenuator : public Module
{
public:
    Attenuator(Memory * memory, TransceiverCallback transceiver_cb, void * transceiver, float atten) :
        Module(memory, transceiver_cb, transceiver),
        atten(atten)
    {
    }

    const char * name() {
        static char __ATTENUATOR_NAME__[] = "Attenuator";
        return __ATTENUATOR_NAME__;
    }

private:
    float atten;

    Block * process(Block * in)
    {
        return new AttenuatorBlock(in, atten);
    }
};

#endif
