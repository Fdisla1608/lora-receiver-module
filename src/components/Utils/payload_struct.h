#ifndef PAYLOAD_STRUCT_H
#define PAYLOAD_STRUCT_H
#include <Arduino.h>

struct Payload
{
    uint8_t id;
    uint8_t type;
    uint8_t data[255];
};

#endif