#include <math.h>
#include <string.h>
#include "Constellation.h"
#include "../CostasLoop/CostasLoopBlock.h"

static const char __CONSTELLATION_NAME__[] = "Constellation";

const char * Constellation::name() {
    return __CONSTELLATION_NAME__;
}

Constellation::Constellation(Memory * memory,
        TransceiverCallback cb,
        void * trans,
        double fs,
        size_t chunk):
    Module(memory, cb, trans),
    fs(fs),
    chunk(chunk),
    in_phase_queue((1 << 18)),
    qu_phase_queue((1 << 18))
{
    update_interval = (int) round(100.0*chunk/fs);
    in_phase_memory = new float[chunk];
    qu_phase_memory = new float[chunk];
    memset(in_phase_memory, 0, sizeof(float) * chunk);
    memset(qu_phase_memory, 0, sizeof(float) * chunk);
}

int Constellation::get_updateInterval()
{
    return update_interval;
}

Constellation::~Constellation()
{
    delete [] in_phase_memory;
    delete [] qu_phase_memory;
}

Block * Constellation::process(Block * in)
{
    static RC_LowPass no_lock_timer(0.25, fs);
    static char errors[][25] = 
    {
        {"No Error"},
        {"Queue full"}
    };
    int error = 0;

    CostasLoopBlock * qpsk = (CostasLoopBlock *) in;

    float * in_phase = qpsk->get_pointer(IN_PHASE_SIGNAL);
    float * qu_phase = qpsk->get_pointer(QU_PHASE_SIGNAL);
    float * lock_sig = qpsk->get_pointer(LOCK_SIGNAL);

    bool in_res;
    bool qu_res;

    Block * ret = memory->allocate(in->get_size());
    float ** ret_iter = ret->get_iterator();

    qpsk->reset();
    ret->reset();
 
    do
    {
        in_res = in_phase_queue.add(*in_phase);
        qu_res = qu_phase_queue.add(*qu_phase);
        if ((!in_res) || (!qu_res))
        {
            error = 1;
            goto fail;
        }

        if (*lock_sig < 0.8)
        {
            no_lock_timer.work(1.0);

            if (no_lock_timer.value() > 0.99)
            {
                LOG("Hard Resetting Demodulator\n");
                qpsk->hard_reset();
                no_lock_timer.reset();

                RadioMsg reset_notify(NOTIFY_PLL_RESET);
                qpsk->costa->broadcast(&reset_notify);
            }
        }
        **ret_iter = *in_phase + *qu_phase;
    } while(qpsk->next());

    in->free();
    return ret;

fail:
    in->free();
    ret->free();
    ERROR("%s\n", errors[error]);
    return NULL;
}

size_t Constellation::size()
{
    return chunk;
}

Point Constellation::get_data(size_t index)
{
    Point p;
    p.x = in_phase_memory[index];
    p.y = qu_phase_memory[index];
    return p;
}

void Constellation::next()
{

}

Point Constellation::get_origin()
{
    Point p;
    p.x = -1.5;
    p.y = -1.5;
    return p;
}

Point Constellation::get_lengths()
{
    Point p;
    p.x = 3.0;
    p.y = 3.0;
    return p;
}

bool Constellation::valid()
{
    bool valid = !(qu_phase_queue.size() >= chunk && in_phase_queue.size() >= chunk);

    if (!valid)
    {
        (void) in_phase_queue.get(in_phase_memory, chunk);
        (void) qu_phase_queue.get(qu_phase_memory, chunk);
        return false;
    }
    return true;
}
