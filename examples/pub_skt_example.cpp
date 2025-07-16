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
    uint16_t port_no = ipc_struct->netskt.port;
    uint32_t ip_addr = ipc_struct->netskt.ipAddr;

    int sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sock_fd == -1) {
        std::cout << "Error\n";
        return 0;
    }
    struct sockaddr_in self_addr;
    self_addr.sin_family = AF_INET;
    self_addr.sin_port = htons(port_no);
    self_addr.sin_addr.s_addr = htonl(ip_addr);

    if(bind(sock_fd, (struct sockaddr *)&self_addr, sizeof(struct sockaddr_in))) {
        std::cout << "bind failed\n";
        close(sock_fd);
        exit(1);
    }

    int rc;
    dmsg_t dmsg;
    dispatcherRegister(sock_fd, "Pub1", PUB_TO_DISPATCH);
    rc = recvfrom(sock_fd, (char*)&dmsg, sizeof(dmsg), 0, NULL, NULL);
    std::cout << "Publisher is registerd with ID " << dmsg.id.publisherId << std::endl;

    int pub_id = dmsg.id.publisherId;
    
    /* ********** Do the stuff here ********** */
    publisherPublish(sock_fd, pub_id, 100);
    std::cout << "Press any key to publish the message\n";
    getchar();
    dmsg_t *data_msg = dmsgDataPrepare2(PUB_TO_DISPATCH, SUB_MSG_DATA, 100, TLV_OVERHEAD_SIZE + tlvDataLen(TLV_DATA_128));
    data_msg->id.publisherId = pub_id;
    data_msg->priority = DMSG_PR_HIGH;
    data_msg->refCount = 1;
    tlvBufferInsertTlv(data_msg->tlvBuffer, TLV_DATA_128, tlvDataLen(TLV_DATA_128), (char*)"Test Data from Pub1");
    pubSubDispatchMsg(sock_fd, data_msg);
    
    std::cout << "Press any key to unpublish\n";
    getchar();
    publisherUnPublish(sock_fd, pub_id, 100);
    /* ********** Do the stuff here ********** */

    std::cout << "Press any key to Unregister the publisher\n";
    getchar();
    dispatcherUnregister(sock_fd, pub_id, PUB_TO_DISPATCH);
    close(sock_fd);
}