#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include <errno.h>
#include <arpa/inet.h>
#include "../common/dmsgOp.h"
#include "../common/comm_type.h"
#include "../common/ipc_struct.h"
#include "../clientLib/client.h"

#define PUB_SKT_UDP_PORT_NO 50000

void *pubSktExample(void *_ipc_struct) {
    ipcStruct_t *ipc_struct = (ipcStruct_t *)_ipc_struct;
    PublisherClient *pubClient = new PublisherClient(ipc_struct);

    pubClient->dispatcherRegister("Pub1", PUB_TO_DISPATCH);
    char buff[1024];
    int rc = pubClient->receiveFrom(buff, sizeof(buff));
    if(rc < 0) {
        std::cout << "Error receiving from Dispatcher\n";
        exit(1);
    }
    dmsg_t *dmsg = (dmsg_t*)buff;
    std::cout << "Publisher is registerd with ID " << dmsg->id.publisherId << std::endl;

    int pub_id = dmsg->id.publisherId;
    
    /* ********** Do the stuff here ********** */
    pubClient->publisherPublish(pub_id, 100);
    std::cout << "Press any key to publish the message\n";
    getchar();
    dmsg_t *data_msg = dmsgDataPrepare2(PUB_TO_DISPATCH, SUB_MSG_DATA, 100, TLV_OVERHEAD_SIZE + tlvDataLen(TLV_DATA_128));
    data_msg->id.publisherId = pub_id;
    data_msg->priority = DMSG_PR_HIGH;
    data_msg->refCount = 1;
    tlvBufferInsertTlv(data_msg->tlvBuffer, TLV_DATA_128, tlvDataLen(TLV_DATA_128), (char*)"Test Data from Pub1");
    pubClient->pubSubDispatchMsg(data_msg);
    
    std::cout << "Press any key to unpublish\n";
    getchar();
    pubClient->publisherUnPublish(pub_id, 100);
    /* ********** Do the stuff here ********** */

    std::cout << "Press any key to Unregister the publisher\n";
    getchar();
    pubClient->dispatcherUnregister(pub_id, PUB_TO_DISPATCH);
    
}