#include "FirFilter.h"
#include <string.h>

FirFilter::FirFilter(Memory * memory, 
            TransceiverCallback cb, 
            void * transceiver, 
            size_t n, 
            const double coefs[]):
    Module(memory, cb, transceiver),
    coefs(coefs),
    n(n)
{
    save = new float[n];
    memset(save, 0, sizeof(float) * n);
}

FirFilter::~FirFilter()
{
    delete [] save;
}

void FirFilter::reset()
{
    memset(save, 0, sizeof(float) * n);
}

float FirFilter::work(float input)
{
    size_t k;
    double output;

    save[0] = input;
    output = 0.0f;

    for (k = 0; k < n; k++)
    {
        output += coefs[k] * save[k];
    }

    for (k = n; k > 1; k--)
    {
        save[k-1] = save[k-2];
    }

    return (float) output;
}

const char * FirFilter::name()
{
    static const char __name__[] = "FirFilter";
    return __name__;
}

class FirFilterBlock : public Block
{
public:
    FirFilterBlock(Block * block, FirFilter * filter):
        block(block),
        filter(filter)
    {
        iter = block->get_iterator();
        ptr = &value;
        block->reset();
        value = filter->work(**iter);
    }

    ~FirFilterBlock() {};

    void free() 
    {
        block->free();
        delete this;
    }

    bool is_free() 
    {
        return block->is_free();
    }

    size_t get_size() 
    {
        return block->get_size();
    }

    void reset() 
    {
        block->reset();
    }

    bool next() 
    {
        bool has_next = block->next();
        if (has_next) 
        {
            value = filter->work(**iter);
            return true;
        }
        return false;
    }

    float ** get_iterator() 
    {
        return &ptr;
    }

    void print() {
        printf("%s Block{\n", filter->name());
        block->print();
        printf("}\n");
    }

public:
    Block * block;
    FirFilter * filter;
    float value;
    float * ptr;
    float ** iter;
};

Block * FirFilter::process(Block * block)
{
    return new FirFilterBlock(block, this);
}

