#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <errno.h>
#include <arpa/inet.h>
#include "../Libs/tlv.h"
#include "client.h"

static int ipcTypeToTlvType(ipcType_t ipc_type) {
    switch (ipc_type) {
        case IPC_TYPE_MSGQ:
            return TLV_IPC_TYPE_MSGQ;
        case IPC_TYPE_UXSKT:
            return TLV_IPC_TYPE_UXSKT;
        case IPC_TYPE_NETSKT:
            return TLV_IPC_NET_UDP_SKT;
        case IPC_TYPE_CBK:
            return TLV_IPC_TYPE_CBK;
        default:
            return 0;
    }
}

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

/* below two APIs update pubDB only */
void publisherPublish(int sock_fd, uint32_t pub_id, uint32_t msg_id) {
    dmsg_t *msg = (dmsg_t*)calloc(1, sizeof(*msg));
    msg->msgId = 0;
    msg->msgType = PUB_TO_DISPATCH;
    msg->subMsgType = SUB_MSG_ADD;
    msg->msgCode = msg_id;
    msg->id.publisherId = pub_id;
    msg->id.subscriberId = pub_id;
    msg->tlvBufferSize = 0;

    int rc = pubSubDispatchMsg(sock_fd, msg);
    if(rc < 0) {
        std::cout << "Client Error: Send failed with errno: " << errno << std::endl;
    }
    free(msg);
}

void publisherUnPublish(int sock_fd, uint32_t pub_id, uint32_t msg_id) {
    dmsg_t *msg = (dmsg_t*)calloc(1, sizeof(*msg));
    msg->msgId = 0;
    msg->msgType = PUB_TO_DISPATCH;
    msg->subMsgType = SUB_MSG_DELETE;
    msg->msgCode = msg_id;
    msg->id.publisherId = pub_id;
    msg->id.subscriberId = pub_id;
    msg->tlvBufferSize = 0;

    int rc = pubSubDispatchMsg(sock_fd, msg);
    if(rc < 0) {
        std::cout << "Client Error: Send failed with errno: " << errno << std::endl;
    }
    free(msg);
}

/* below two APIs update subDB and pubSubDB */
void subscriberSubscribe(int sock_fd, uint32_t sub_id, uint32_t msg_id) {
    dmsg_t *msg = (dmsg_t *)calloc (1, sizeof(*msg));
    msg->msgId = 0;
    msg->msgType = SUB_TO_DISPATCH;
    msg->subMsgType = SUB_MSG_ADD;
    msg->msgCode = msg_id;
    msg->id.publisherId = sub_id;
    msg->id.subscriberId = sub_id;
    msg->tlvBufferSize = 0;

    int rc = pubSubDispatchMsg(sock_fd, msg);
    if (rc < 0) {
        printf ("Client Error : Send Failed, errno = %d\n", errno);
    }
    free(msg);
}

void subscriberUnSubscribe(int sock_fd, uint32_t sub_id, uint32_t msg_id) {
    dmsg_t *msg = (dmsg_t *)calloc (1, sizeof (*msg));
    msg->msgId = 0;
    msg->msgType = SUB_TO_DISPATCH;
    msg->subMsgType = SUB_MSG_DELETE;
    msg->msgCode = msg_id;
    msg->id.publisherId = sub_id;
    msg->id.subscriberId = sub_id;
    msg->tlvBufferSize = 0;

    int rc = pubSubDispatchMsg(sock_fd, msg);
    if (rc < 0) {
        printf ("Client Error : Send Failed, errno = %d\n", errno);
    }
    free(msg);
}

void subscriberSubscribeIpcChannel(int sock_fd, uint32_t sub_id, ipcType_t ipc_type, ipcStruct_t *ipc_struct) {
    int ipc_tlv = ipcTypeToTlvType(ipc_type);
    if(!ipc_tlv) {
        std::cout << "Client Error: Invalid IPC Type\n";
        return;
    }
    uint8_t tlv_size = tlvDataLen(ipc_tlv);
    dmsg_t *subscriber_ipc_msg = dmsgDataPrepare2(SUB_TO_DISPATCH, SUB_MSG_IPC_CHANNEL_ADD, 0, TLV_OVERHEAD_SIZE + tlv_size);
    subscriber_ipc_msg->id.subscriberId = sub_id;
    subscriber_ipc_msg->msgId = 0; // This will be assigned by the Dispatcher
    char *tlv_buffer = (char *)subscriber_ipc_msg->tlvBuffer;
    uint8_t tlv_buffer_len = subscriber_ipc_msg->tlvBufferSize;
    tlvBufferInsertTlv(tlv_buffer, ipc_tlv, tlv_size, NULL);
    char *ipc_tlv_value = tlv_buffer + TLV_OVERHEAD_SIZE;
    switch (ipc_tlv) {
        case TLV_IPC_NET_UDP_SKT: {
            uint32_t *ip_addr = (uint32_t *)(ipc_tlv_value);
            *ip_addr = htonl(ipc_struct->netskt.ipAddr);
            uint16_t *port = (uint16_t *)(ipc_tlv_value + 4);
            *port = htons(ipc_struct->netskt.port);
        }
        break;

        case TLV_IPC_TYPE_CBK: {
            pubSubCbk_t *cbk = (pubSubCbk_t *)ipc_tlv_value;
            *cbk = ipc_struct->cbk.cbk;
        }
        break;

        case TLV_IPC_TYPE_MSGQ: {
            char *msgq_name = (char *)ipc_tlv_value;
            strncpy (msgq_name, ipc_struct->msgq.MsgQName, 
                sizeof(ipc_struct->msgq.MsgQName));
        }
        break;

        case TLV_IPC_TYPE_UXSKT: {
            char *uxskt_name = (char *)ipc_tlv_value;
            strncpy(uxskt_name, ipc_struct->uxskt.UnixSktName, sizeof(ipc_struct->uxskt.UnixSktName));
        }
        break;

        default:
            std::cout << "Client Error : Invalid IPC Type\n";
            free(subscriber_ipc_msg);
            return;
    }
    int rc = pubSubDispatchMsg(sock_fd, subscriber_ipc_msg);
    if (rc < 0) {
        std::cout << "Client Error : Send Failed, errno = " << errno << std::endl;
    }
    free(subscriber_ipc_msg);
}