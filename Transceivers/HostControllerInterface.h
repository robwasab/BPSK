#ifndef __HOST_CONTROLLER_INTERFACE_H__
#define __HOST_CONTROLLER_INTERFACE_H__

class HostControllerInterface
{
    virtual send(uint8_t cmd, uint8_t * data, size_t len) = 0;
    virtual process(uint8_t cmd, uint8_t * data, size_t len) = 0;
};

#endif
