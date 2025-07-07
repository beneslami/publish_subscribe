#include "../common/dmsgOp.h"
#include "../Libs/tlv.h"
#include "dispatchDB.h"
#include "pubsub.h"
#include <iostream>
#include <assert.h>

static uint32_t dispatchGenerateId() {
    static uint32_t id = 0;
    return ++id;
}

dmsg_t *dispatcherProcessPublisherMsg(dmsg_t *msg, uint32_t bytes_read) {
    assert(msg->msgType == PUB_TO_DISPATCH);
    msg->msgId = dispatchGenerateId();
    switch (msg->subMsgType) {
    case SUB_MSG_REGISTER: {
        char *tlv_buffer = (char*)msg->tlvBuffer;
        size_t tlv_buffer_size = msg->tlvBufferSize;
        uint8_t tlv_data_len = 0;
        char *pub_name = tlvBufferGetParticularTlv(
            tlv_buffer, tlv_buffer_size, TLV_CODE_NAME, &tlv_data_len
        );
        if (!pub_name) {
            std::cout << "Dispatcher Error:  Publisher Registration - Publisher name problem\n";
            return dmsgDataPrepare2(DISPATCH_TO_PUB, SUB_MSG_ERROR, ERROR_TLV_MISSING, 0);
        }
        publisherDBentry_t *pubEntry = publisherDbCreate(dispatchGenerateId(), pub_name);
        dmsg_t *reply_msg = dmsgDataPrepare2(DISPATCH_TO_PUB, SUB_MSG_ID_ALLOC_SUCCESS, 0, 0);
        reply_msg->id.publisherId = pubEntry->publisherId;
        std::cout << "Dispatcher: New Publisher registered with Pub ID " << pubEntry->publisherId << std::endl;
        return reply_msg;
    }
    break;
    
    case SUB_MSG_UNREGISTER: {
        publisherDbDelete(msg->id.publisherId);
        std::cout << "Dispatcher : Publisher id " << msg->id.publisherId << " Un-Registered\n";
        
    }
    break;

    default:
        break;
    }
    return NULL;
}

dmsg_t *dispatcherProcessSubscriberMsg(dmsg_t *msg, uint32_t bytes_read) {
    dmsg_t *reply_msg;
    assert(msg->msgType == SUB_TO_DISPATCH);
    msg->msgId = dispatchGenerateId();
    switch (msg->subMsgType) {
        case SUB_MSG_ADD:
        {
            bool rc = subscriberUnsubscribeMsg(msg->id.subscriberId, msg->msgCode);
            if (!rc) {
                std::cout << "Dispatcher Error : New Msg Subscribing Failed by Subscriber ID " << msg->id.subscriberId << std::endl;
            }
        }
        break;
        case SUB_MSG_DELETE:
        {
            bool rc = subscriberUnsubscribeMsg(msg->id.subscriberId, msg->msgCode);
        }
        break;
        case SUB_MSG_REGISTER:
        {
            /* New Subscriber Registration */
            char *tlv_buffer = (char *)msg->tlvBuffer;
            size_t tlv_bufer_size = msg->tlvBufferSize;
            uint8_t tlv_data_len = 0;
            char *sub_name = tlvBufferGetParticularTlv(tlv_buffer, tlv_bufer_size, TLV_CODE_NAME, &tlv_data_len);
            if (!sub_name) {
                std::cout << "Dispatcher Error : Subscriber Registration : Subscriber Name TLV Missing\n";
                return dmsgDataPrepare2(DISPATCH_TO_SUB, SUB_MSG_ERROR, ERROR_TLV_MISSING, 0);
            }

            std::shared_ptr<subscriberDBentry_t> SubEntry = subscriberDbCreate(dispatchGenerateId() , sub_name);
            dmsg_t *reply_msg = dmsgDataPrepare2(DISPATCH_TO_SUB, SUB_MSG_ID_ALLOC_SUCCESS, 0, 0);
            reply_msg->id.subscriberId = SubEntry->subscriberId;
            return reply_msg;
        }
        break;
        case SUB_MSG_UNREGISTER:
        {
            subscriberDbDelete(msg->id.subscriberId);
        }
        break;
        case SUB_MSG_REQUEST_ACK:
        break;
        case SUB_MSG_ACK_MSG:
        break;
        case SUB_MSG_SUBCRIBER_LIST:
        break;
        case SUB_MSG_INFORM_NEW_SUBS:
        break;
        case SUB_MSG_IPC_CHANNEL_ADD: {
            bool rc = dispatcherProcessSubscriberIpcSubscription(msg->id.subscriberId, msg);
        }
        break;
        default:;
    }
    return NULL;
}