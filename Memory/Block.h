#ifndef __BLOCK_H__
#define __BLOCK_H__

#include <stdlib.h>
#include <stdio.h>
#include "../Colors/Colors.h"

class Block
{
public:
    virtual void free() = 0;
    virtual bool is_free() = 0;
    virtual size_t get_size() = 0;
    virtual void reset() = 0;
    virtual bool next() = 0;
    virtual float ** get_iterator() = 0;
    virtual void print() = 0;
    virtual ~Block() {};
};

class MemoryBlock : public Block
{
public:
    MemoryBlock() 
    {
        head = 0;
        size = 0;
        memory = NULL;
        memory_size = 0;
        ptr = NULL;
        index = 0;
        end = 0;
        _free = false;
        counter = 0;
    }

    ~MemoryBlock() {}

    MemoryBlock(size_t head, size_t size, float * memory, size_t memory_size) :
        head(head),
        size(size),
        memory(memory),
        memory_size(memory_size)
    {
        if (size > 0) {
            ptr = &memory[head];
            index = head;
            end = (head + size) % memory_size;
            _free = false;
            counter = 1;
        }
        else {
            ptr = NULL;
            index = head;
            end = head;
            counter = 0;
        }
    }

    void free()
    {
        _free = true;
    }

    bool is_free()
    {
        return _free;
    }

    size_t get_size()
    {
        return size;
    }

    void reset()
    {
        ptr = &memory[head];
		index = head;
        counter = 1;
    }

    bool next()
    {
        if (counter >= size)
        {
            //RED;
            //printf("NO MORE DATA!\n");
            //ENDC;
            return false;
        }
        index = (index + 1) % memory_size;
        ptr = &memory[index];
        counter += 1;
        return true;
    }

    float ** get_iterator() {
        return &ptr;
    }

    void print() {
        DIM;
        printf("   size  : %zu\n", size);
        printf("   head  : %zu\n", head);
        printf("   end   : %zu\n", end);
        printf("   freed : %s\n", _free ? "yes" : "no");
        printf("   ptr   : %p\n", ptr);
        printf("   &ptr  : %p\n", &ptr);
        ENDC;
    }

private:
    size_t head;
    size_t size;
    float * memory;
    size_t memory_size;
    float * ptr;
    size_t index;
    size_t end;
    bool _free;
    size_t counter;
};

#endif
