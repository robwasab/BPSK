#ifndef __STACK_H__
#define __STACK_H__

template<class Type>
class Stack
{
public:
    Stack(size_t max): _max(max)
    {
        printf("Allocating Stack data: %zu elements\n", _max);
        _data = new Type[_max];
        _tail = 0;
        _size = 0;
        pthread_mutex_init(&mutex, NULL);
    }

    ~Stack()
    {
        printf("Deleteing stack data...\n");
        delete [] _data;
    }

    Type * peek()
    {
        Type * ret;
        lock();
        if (_size > 0)
        {
            ret = &_data[_tail - 1];
        }
        else
        {
            ret = NULL;
        }
        unlock();
        return ret;
    }

    Type * push(Type& t)
    {
        Type * ret;
        lock();

        if (_size < _max)
        {
            // copy onto stack
            _data[_tail ] = t;
            ret = &_data[_tail ];
            _tail += 1;
            _size += 1;
            unlock();
            return ret;
        }
        else
        {
            unlock();
            return NULL;
        }
    }

    int pop(Type * t)
    {
        lock();

        if (_size > 0)
        {
            *t = _data[_tail - 1 ];
            _tail -= 1;
            _size -= 1;
            unlock();
            return 0;
        }
        else 
        {
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

private:
    const size_t _max;
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
