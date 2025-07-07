#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include <errno.h>
#include <arpa/inet.h>
#include "../Libs/tlv.h"
#include "client.h"

int pubSubDispatchMsg(int sock_fd, dmsg_t *dmsg) {
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DISPATCHER_UDP_PORT);
    server_addr.sin_addr.s_addr = htonl(DISPATCHER_IP_ADDR);
    int rc = sendto(sock_fd, (char*)dmsg, sizeof(*dmsg) + dmsg->tlvBufferSize, 0, (struct sockaddr*)&server_addr, sizeof(struct sockaddr));
    return rc;
}

void dispatcherRegister(int sock_fd, const char *entitiy_name, msgType_t msg_type) {
    dmsg_t *dmsg = (dmsg_t*)calloc(1, sizeof(*dmsg) + TLV_OVERHEAD_SIZE + TLV_CODE_NAME_LEN);
    dmsg->msgId = 0;
    dmsg->msgType = msg_type;
    dmsg->subMsgType = SUB_MSG_REGISTER;
    dmsg->id.publisherId = 0;
    dmsg->id.subscriberId = 0;
    dmsg->tlvBufferSize = TLV_OVERHEAD_SIZE + TLV_CODE_NAME_LEN;
    char *tlv_buffer = (char*)dmsg->tlvBuffer;
    tlvBufferInsertTlv(tlv_buffer, TLV_CODE_NAME, TLV_CODE_NAME_LEN, (char*)entitiy_name);
    int rc = pubSubDispatchMsg(sock_fd, dmsg);
    if(rc < 0) {
        std::cout << "Error sending to The dispatcher\n";
    }
    free(dmsg);
}

void dispatcherUnregister(int sock_fd, uint32_t pub_id, msgType_t msg_type) {
    dmsg_t *dmsg = (dmsg_t*)calloc(1, sizeof(*dmsg));
    dmsg->msgId = 0;
    dmsg->msgType = msg_type;
    dmsg->subMsgType = SUB_MSG_UNREGISTER;
    dmsg->id.publisherId = pub_id;
    dmsg->id.subscriberId = pub_id;
    dmsg->tlvBufferSize = 0;
    
    int rc = pubSubDispatchMsg(sock_fd, dmsg);
    if(rc < 0) {
        std::cout << "Client Error: Send failed - errno = " << errno << std::endl;
    }
    free(dmsg);
}