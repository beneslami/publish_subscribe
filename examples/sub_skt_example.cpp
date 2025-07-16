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
    int sock_fd;
    static char buffer[1024];
    ipcStruct_t *ipc_struct = (ipcStruct_t *)_ipc_struct;
    sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_fd == -1) {
        printf ("Error : Socket Creation Failed\n");
        return 0;
    }
    struct sockaddr_in self_addr;
    self_addr.sin_family = AF_INET;
    self_addr.sin_port = htons(ipc_struct->netskt.port);
    self_addr.sin_addr.s_addr = htonl(ipc_struct->netskt.ipAddr);
    if (bind(sock_fd, (struct sockaddr *)&self_addr, sizeof(struct sockaddr)) == -1) {
        printf("Dispatcher Error : bind failed - error: %u\n", errno);
        close (sock_fd);
        exit(1);
    }
    int rc; 
    dmsg_t dmsg;
    dispatcherRegister(sock_fd, "Sub1", SUB_TO_DISPATCH);
    rc = recvfrom (sock_fd, (char *)&dmsg, sizeof (dmsg), 0, NULL, NULL);
    std::cout << "Sub Msg ID allocated = " << dmsg.id.subscriberId << std::endl;
    int sub_id = dmsg.id.subscriberId;

    /* ********** Do the stuff here ********** */
    subscriberSubscribe(sock_fd, sub_id, 100);
    ipcStruct_t ipc_struct2;
    ipc_struct2.netskt.ipAddr = INADDR_ANY;
    ipc_struct2.netskt.port = htons(self_addr.sin_port);
    std::cout << "press any key to report IPC channel socket to the dispatcher\n";
    getchar();
    subscriberSubscribeIpcChannel(sock_fd, sub_id, IPC_TYPE_NETSKT, &ipc_struct2);
    while (1) {
        std::cout << "Subscriber now waiting for msgs from Dispatcher\n";
        rc = recvfrom (sock_fd, (char *)buffer, sizeof(buffer), 0, NULL, NULL);
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
        std::cout << "Data Message recvd by Subscriber " << "[" << sub_id << "] is : " << tlv_value << "\n\n";
        //dmsgDebugPrint(recv_msg);
    }

    std::cout << "Press any key to unsubscribe\n";
    getchar();
    subscriberUnSubscribe(sock_fd, sub_id, 100);
    /* ********** Do the stuff here ********** */
    
    close(sock_fd);
    return 0;
}