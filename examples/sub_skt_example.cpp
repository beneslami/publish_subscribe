#include <stdlib.h>
#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include "../clientlib/client.h"
#include "../common/ipc_struct.h"

void *subSktExample(void *_ipc_struct) {
    ipcStruct_t *ipc_struct = (ipcStruct_t*)_ipc_struct;
    SubscriberClient *subClient = new SubscriberClient(ipc_struct);
    
    subClient->dispatcherRegister("Sub1", SUB_TO_DISPATCH);
    char buff[1024];
    int rc = subClient->receiveFrom(buff, sizeof(buff));
    if(rc < 0) {
        std::cout << "Error receiving from Dispatcher\n";
        exit(1);
    }
    dmsg_t *dmsg = (dmsg_t*)buff;
    std::cout << "Sub Msg ID allocated = " << dmsg->id.subscriberId << std::endl;
    int sub_id = dmsg->id.subscriberId;

    /* ********** Do the stuff here ********** */
    subClient->subscriberSubscribe(sub_id, 100);
    ipcStruct_t ipc_struct2;
    ipc_struct2.netskt.ipAddr = ipc_struct->netskt.ipAddr;
    ipc_struct2.netskt.port = ipc_struct->netskt.port;
    std::cout << "press any key to report IPC channel socket to the dispatcher\n";
    getchar();
    subClient->subscriberSubscribeIpcChannel(sub_id, IPC_TYPE_NETSKT, &ipc_struct2);
    static char buffer[1024];
    while (1) {
        std::cout << "Subscriber now waiting for msgs from Dispatcher\n";
        int rc = subClient->receiveFrom(buffer, sizeof(buffer));
        if(rc < 0) {
            std::cout << "Error receiveing from the Dispatcher\n";
            exit(1);
        }
        dmsg_t *recv_msg = (dmsg_t *)buffer;
        std::cout << "Subscriber : Msg recvd from Dispatcher\n";
        char *tlv_buffer = recv_msg->tlvBuffer;
        uint16_t tlv_buffer_size = recv_msg->tlvBufferSize;
        uint8_t tlv_data_len;
        char *tlv_value = tlvBufferGetParticularTlv(tlv_buffer, tlv_buffer_size, TLV_DATA_128, &tlv_data_len);
        if (!tlv_value) {
            std::cout << "Error : No TLV_DATA found in TLV buffer\n";
            continue;
        }
        std::cout << "Data Message received by Subscriber " << "[" << sub_id << "] is : " << tlv_value << "\n\n";
        //dmsgDebugPrint(recv_msg);
    }

    std::cout << "Press any key to unsubscribe\n";
    getchar();
    subClient->subscriberUnSubscribe(sub_id, 100);
    /* ********** Do the stuff here ********** */
    
    return 0;
}