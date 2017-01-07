#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../Colors/Colors.h"

template<class Type> 
class Queue
{
public:
    Queue(size_t max) : _max(max)
    {
        printf("Allocating Queue data: %zu elements\n", _max);

        _data = new Type[_max];
        _head = 0;
        _tail = 0;
        _size = 0;
        pthread_mutex_init(&mutex, NULL);
    }

    ~Queue()
    {
        printf("Deleting queue data...\n");
        delete [] _data;
    }

    Type * add(Type& t)
    {
        // printf("Adding to the QUEUE!\n");
        Type * ret;
        lock();

        if (_size < _max) 
        {
            _data[ _tail ] = t;
            ret = &_data[ _tail ];
            _tail = ( _tail + 1) % _max;
            _size = _size + 1;
            unlock();
            return ret;
        }
        else 
        {
            unlock();
            // printf("Queue is full!\n");
            return NULL;
        }
    }

    int add(Type * t, size_t len)
    {
        lock();

        if (_size + len < _max) 
        {
            for (size_t n = 0; n < len; ++n ) 
            {
                _data[ (_tail + n) % _max ] = t[n];
            }
            _tail = ( _tail + len ) % _max;
            _size += len;
            unlock();
            return 0;
        }
        else 
        {
            RED;
            printf("Queue is full!\n");
            ENDC;
            unlock();
            return -1;
        }
    }

    int get(Type * t)
    {
        lock();

        if (_size > 0) 
        {
            *t = _data[ _head ];
            _head = ( _head + 1 ) % _max;
            _size = _size - 1;
            //printf("Queue size: %zu, head: %zu, tail %zu\n", _size, _head, _tail);
            unlock();
            return 0;
        }
        else 
        {
            // printf("Empty...\n");
            unlock();
            return -1;
        }
    }

    int get(Type * t, size_t len)
    {
        lock();
        if (_size > len)
        {
            for (int n = 0; n < len; ++n)
            {
                t[n] = _data[( _head + n ) % _max];
            }
            _head = ( _head + len ) % _max;
            _size -= len;
            unlock();
            return 0;
        }
        else
        {
            // printf("Empty... Requested length: %zu/%zu\n", len, _size);
            unlock();
            return -1;
        }
    }

    size_t size()
    {
        size_t size;
        lock();
        size = _size;
        unlock();
        return size;
    }

    Type * peek()
    {
        Type * ret;
        lock();
        ret = &_data[_head ];
        unlock();
        return ret;
    }

private:
    const size_t _max;
    size_t _head;
    size_t _tail;
    size_t _size;
    Type * _data;
    pthread_mutex_t mutex;

    int lock() {
        pthread_mutex_lock(&mutex);
        return 0;
    }

    int unlock() {
        pthread_mutex_unlock(&mutex);
        return 0;
    }
};

#endif
