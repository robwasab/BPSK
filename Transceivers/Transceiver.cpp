#include "Transceiver.h"
#include "../MaximumLength/generator.h"
#include "../CRC-16/crc.h"


#define MAX_MODULES 32

/* Radio modules use this function, which is passed as a function pointer
 * into their constructor, to send messages back to the transceiver.
 */
void transceiver_callback(void * arg, RadioMsg * msg)
{
    Transceiver * self = (Transceiver *) arg;

    self->notify(*msg);
}

/* Ignore, was used for debuging the signaling mechanism
 */
void Transceiver::debug(RadioMsg msg)
{
    signal();
}

Transceiver::Transceiver(TransceiverNotify notify_cb, void * obj, 
        double fs,
        double ftx, 
        double frx, 
        double fif, 
        double bw,
        int cycles_per_bit,
        PlotController * controller):
    SignaledThread(128),
    notify_cb(notify_cb),
    obj(obj),
    fs(fs),
    ftx(ftx),
    frx(frx),
    fif(fif),
    bw(bw),
    cycles_per_bit(cycles_per_bit),
    order(6),
    spectrum_size(1 << 10),
    controller(controller)
{
    generate_ml_sequence(&prefix_len, &prefix);

    crc_table = compute_crc_table(); 

    /* Initialize Objects */
    tx_memory = new Memory();
    rx_memory = new Memory();
    modules = new Module*[MAX_MODULES];
    sources = new DataSource*[MAX_MODULES];
    
    int k;
    for (k = 0; k < MAX_MODULES; k++)
    {
        modules[k] = NULL;
        sources[k] = NULL;
    }
}

void Transceiver::process(RadioMsg msg)
{
    RadioData * data = (RadioData *) &msg;
    int k;

    switch (msg.type)
    {
        case PROCESS_DATA:
            if (modules[data->get_id() + 1] != NULL)
            {
                modules[data->get_id() + 1]->dispatch(&msg);
            }
            else
            {
                if (data->get_block() != NULL)
                {
                    LOG("No module to handle block...\n");
                    data->get_block()->free();
                }
            }
            break;

        case NOTIFY_CRC_CORRUPTED:
        case NOTIFY_DATA_START:
        case NOTIFY_DATA_BODY:
        case NOTIFY_USER_REQUEST_QUIT:
            notify_cb(obj, &msg);
            break;

        case NOTIFY_PLL_RESET:
        case NOTIFY_PLL_LOST_LOCK:
            notify_cb(obj, &msg);
            // fall through
        case NOTIFY_PLL_LOCK:
        case CMD_START:
        case CMD_STOP:
        case CMD_RESET_ALL:
        case CMD_RESET_TRANSMITTER:
        case CMD_RESET_RECEIVER:
        case CMD_SET_TRANSMIT_CHANNEL:
        case CMD_SET_RECEIVE_CHANNEL:
        case CMD_SET_NOISE_LEVEL:
        case CMD_TEST_PSK8_SIG_GEN:
        case NOTIFY_PACKET_HEADER_DETECTED:
        case NOTIFY_RECEIVER_RESET_CONDITION_DETECTED:
        case NOTIFY_MSG_EXHAUSTED:

            Log("BROADCASTING: %s\n", RadioMsgString[msg.type]);
            k = 0;
            while (modules[k] != NULL)
            {
                //LOG("[%2d]: %s->%s\n", k, RadioMsgString[msg.type], modules[k]->name());
                modules[k]->dispatch(&msg);
                k++;
            }
            break;
        default:
            Log("Unknown message type: %s\n", RadioMsgString[msg.type]);
            break;
    }
}

void Transceiver::start(bool block)
{
    RadioMsg msg(CMD_START);
    this->notify(msg);
    SignaledThread::start(block);
}

void Transceiver::stop()
{
    RadioMsg msg(CMD_STOP);
    this->notify(msg);
    SignaledThread::stop();
}

Transceiver::~Transceiver()
{
    int k;
    
    #ifdef GUI
    for (k = 0; k < MAX_MODULES; k++)
    {
        if (sources[k] != NULL)
        {
            LOG("[%2d]: Removing Source %s\n", k, sources[k]->name());
            controller->remove_plot(sources[k]);
        }
    }
    #endif
    
    LOG("delete [] sources...\n");
    delete [] sources;
    
    for (k = 0; k < MAX_MODULES; k++)
    {
        if (modules[k] != NULL)
        {
            LOG("[%2d]: Deleting Module %s\n", k, modules[k]->name());
            delete modules[k];
        }
    }
    LOG("delete [] modules...\n");
    delete [] modules;

    LOG("delete tx_memory...\n");
    delete tx_memory;

    LOG("delete rx_memory...\n");
    delete rx_memory;

    LOG("delete crc_table...\n");
    delete crc_table;
}

