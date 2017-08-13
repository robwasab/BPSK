#ifndef __BYTE_INPUT_INTERFACE_H__
#define __BYTE_INPUT_INTERFACE_H__

#include "../Module/Module.h"

const char _BYTE_INPUT_INTERFACE_NAME_[] = "ByteInputInterface";

class ByteInputInterface: public Module
{
public:
    ByteInputInterface(Memory * memory, 
            TransceiverCallback cb, 
            void * trans, 
            const uint16_t crc_table[]);
    Block * process(Block * block);
    void process_msg(const uint8_t msg[], size_t len);
    void dispatch(RadioMsg * msg);
    const char * name() 
    {
        return _BYTE_INPUT_INTERFACE_NAME_;
    }
private:
    const uint16_t * crc_table;
};

#endif
