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
        printf("Coordinator : Error : bind failed - error: %u\n", errno);
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
    std::cout << "Press any key to unsubscribe\n";
    getchar();
    subscriberUnSubscribe(sock_fd, sub_id, 100);
    /* ********** Do the stuff here ********** */
    
    close(sock_fd);
    return 0;
}