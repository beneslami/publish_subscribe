#include "../common/ipc_struct.h"

extern void *pubSktExample(void *ipc_struct);

int main (int argc, char **argv) {
    ipcStruct_t ipc_struct;
    ipc_struct.netskt.ipAddr = 2130706433 ; //127.0.0.1
    ipc_struct.netskt.port = 50000;

    pubSktExample((void *)&ipc_struct);
    return 0;
}