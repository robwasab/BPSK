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
    vco = new Integrator(fs);
}

Modulator::~Modulator()
{
    delete vco;
}

class ModulatorBlock : public Block
{
public:
    ModulatorBlock(Block * block, 
            Integrator * vco, 
            double inc):
        block(block),
        vco(vco),
        inc(inc) 
    {
        block_iter = block->get_iterator();
    }

    void free() {
        block->free();
        delete this
    }

    bool is_free() {
        return block->is_free();
    }

    size_t get_size() {
        return block->get_size();
    }

    void reset() {
        block->reset();
        vco->reset();
    }

    bool next() {
        block->next();
        value = 

    float ** get_iterator() = 0;
    void print() = 0;

private:
    Block * block;
    Integrator * vco;
    double inc;
    float value;
    float ** block_iter;
};
