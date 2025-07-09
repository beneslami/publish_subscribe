#pragma once

#include <stdint.h>

typedef struct dmsg_  dmsg_t;
typedef void (*pubSubCbk_t)(dmsg_t *);

typedef union ipcStruct_ {
    struct {
        uint32_t ipAddr;
        uint16_t port;
        uint8_t transportType;
        int sockFd;
    } netskt;

    struct {
        char MsgQName[64];
    } msgq;

    struct {
        char UnixSktName[64];
    } uxskt;

    struct {
        pubSubCbk_t cbk;
    } cbk;
}  ipcStruct_t;