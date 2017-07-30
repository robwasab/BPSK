#include "PlottableAutogain.h"
#include <string.h>


PlottableAutogain::PlottableAutogain(
            Memory * memory, 
            TransceiverCallback cb, 
            void * transceiver, 
            double fs, 
            size_t chunk) :
    Autogain(memory, cb, transceiver, fs),
    chunk(chunk),
    queue((1 << 18))
{
    update_interval = (int) round(chunk/fs);
    plot_memory = new float[chunk];
    memset(plot_memory, 0, sizeof(float) * chunk);
}

int PlottableAutogain::get_updateInterval()
{
    return update_interval;
}

PlottableAutogain::~PlottableAutogain()
{
    delete [] plot_memory;
}

size_t PlottableAutogain::size()
{
    return chunk;
}


Point PlottableAutogain::get_data(size_t index)
{
    Point p;
    p.x = index;
    p.y = plot_memory[index];
    return p;
}

void PlottableAutogain::next()
{

}

Point PlottableAutogain::get_origin()
{
    Point p;
    p.x = 0;
    p.y = -4.0;
    return p;
}

Point PlottableAutogain::get_lengths()
{
    Point p;
    p.x = chunk;
    p.y = 8.0;
    return p;
}

bool PlottableAutogain::valid()
{
    bool valid = !(queue.size() >= chunk);

    if (!valid)
    {
        (void) queue.get(plot_memory, chunk);
        return false;
    }
    return true;
}

float PlottableAutogain::work(float val)
{
    bool res;
    
    val = Autogain::work(val);
    
    res = queue.add(val);

    if (!res)
    {
        ERROR("Queue Full!\n");
    }
    return val;
}

const char * PlottableAutogain::name()
{
    return Autogain::name();
}

