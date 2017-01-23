#include "Modulator.h"
#include <math.h>

const char __name__[] = "Modulator";

const char * Modulator::name()
{
    return __name__;
}

Modulator::Modulator(Memory * memory, 
        Module * next,
        double fs, 
        double fc):
    Module(memory, next)
{
    inc = 2.0 * M_PI * fc/fs;
    phase = 0;
}

Modulator::~Modulator()
{
}

class ModulatorBlock : public Block
{
public:
    ModulatorBlock(Block * block, 
            double inc,
            double * phase):
        block(block),
        inc(inc),
        phase(phase) 
    {
        block_iter = block->get_iterator();
        ptr = &value;
        block->reset();
        value = 2.0 * **block_iter * sin(*phase);
        *phase += inc;
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
            value = 2.0 * **block_iter * sin(*phase);
            //value = sin(*phase);
            *phase += inc;
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
    double inc;
    double * phase;
    float value;
    float * ptr;
    float ** block_iter;
};

Block * Modulator::process(Block * sig)
{
    return new ModulatorBlock(sig, inc, &phase);
}
