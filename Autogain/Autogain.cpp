#include "Autogain.h"
#include <math.h>

const char __name__[] = "Autogain";

const char * Autogain::name() {
    return __name__;
}


Autogain::Autogain(Memory * memory, TransceiverCallback cb, void * trans, double fs):
    Module(memory, cb, trans),
    autogain_c(9.0, 0.01, 44.1E3),
    autogain_b(9.0, 0.10, 44.1E3),
    autogain_a(9.0, 1.00, 44.1E3)
{
}

Autogain::~Autogain()
{
}

class AutogainBlock : public Block
{
public:
    AutogainBlock() :
        autogain(NULL),
        b(NULL),
        iter(NULL),
        ptr(NULL),
        value(0.0),
        _free(false)
    {
    }

    AutogainBlock& operator=(const AutogainBlock& src)
    {
        this->b = src.b;
        this->autogain = src.autogain;
        this->iter = this->b->get_iterator();
        this->ptr = &(this->value);
        _free = false;
        reset();
        return *this;
    }

    AutogainBlock(Block * b, Autogain * autogain) : 
        autogain(autogain),
        b(b) 
    {
        iter = b->get_iterator();
        ptr = &value;
        _free = false;
        b->reset();
        value = autogain->work(**iter);
    }

    ~AutogainBlock()
    {
    }

    void reset() {
        b->reset();
    }

    void free() 
    {
        _free = true;
        b->free();
        delete this;
    }

    bool is_free() 
    {
        return b->is_free();
    }

    size_t get_size() 
    {
        return b->get_size();
    }

    bool next() 
    {
        bool has_next = b->next();
        if (has_next) {
            value = autogain->work(**iter);
            return true;
        }
        return false;
    }

    float ** get_iterator() {
        return &ptr;
    }

    void print() {
        printf("Autogain: %p\n", autogain);
        printf("iterator: %p\n", iter);
        printf("pointer:  %p\n", ptr);
        printf("&value:   %p\n", &value);
        b->print();
    }

private:
    Autogain * autogain;
    Block * b;
    float ** iter;
    float * ptr;
    float value;
    bool _free;
};

float Autogain::work(float val)
{
    return autogain_a.work(autogain_b.work(autogain_c.work( val )));
}

/*
Block * Autogain::process(Block * block)
{
    static char errors[][25] = {
        {"No error"},
        {"Block allocation error"}
    };
    int error = 0;
    float ** iter = NULL;
    float ** out_iter = NULL;
    Block * out = NULL;
    iter = block->get_iterator();
    out = memory->allocate(block->get_size());
    
    if (!out) {
        error = 1;
        goto fail;
    }
    out_iter = out->get_iterator();
    block->reset();
    do
    {
        **out_iter = autogain_a.work(autogain_b.work(autogain_c.work(**iter * 0.07)));
    } while(out->next() && block->next());
    block->free();
    return out;
fail:
    block->free();
    if (out) {
        out->free();
    }
    ERROR("%s\n", errors[error]);
    return NULL;
}
*/


Block * Autogain::process(Block * block) {
    return new AutogainBlock(block, this);
}
