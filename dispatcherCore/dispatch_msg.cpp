#include "../common/dmsgOp.h"


static uint32_t

dispatchGenerateId() {
    static uint32_t id = 0;
    return ++id;
}

dmsg_t *dispatcherProcessPublisherMsg(dmsg_t *msg, uint32_t bytes_read){
    return NULL;
}

dmsg_t *dispatcherProcessSubscriberMsg(dmsg_t *msg, uint32_t bytes_read){
    return NULL;
}