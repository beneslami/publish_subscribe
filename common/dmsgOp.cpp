#include <iostream>
#include "../Libs/tlv.h"
#include "dmsgOp.h"
#include "comm_type.h"

void dmsgDebugPrint(dmsg_t *dmsg) {
    std::cout << "Msg ID: " << dmsg->msgId << " | ";
    std::cout << "Msg Type: " << msgTypeToString(dmsg->msgType) << " | ";
    std::cout << "Sub Msg Type: " << subMsgTypeToString(dmsg->subMsgType) << " | ";
    std::cout << "Msg Code: " << dmsg->msgCode << " | ";
    std::cout << "Publisher ID: " << dmsg->id.publisherId << " | ";
    std::cout << "Subscriber ID: " << dmsg->id.subscriberId << " | ";
    std::cout << "TLV Buffer Size: " << dmsg->tlvBufferSize << " | ";

    char *tlv_buffer = (char*)(dmsg->tlvBuffer);
    size_t tlv_buffer_size = dmsg->tlvBufferSize;
    uint8_t tlv_type = 0;
    uint32_t tlv_len = 0;
    char *tlv_value = NULL;

    ITERATE_TLV_BEGIN(tlv_buffer, tlv_type, tlv_len, tlv_value, tlv_buffer_size) {
        std::cout <<  "TLV Type : "   << tlv_type << " | ";
        std::cout <<  "TLV Length : " << tlv_len << " | ";
        std::cout <<  "TLV Value : "  << tlv_value << " | ";

    } ITERATE_TLV_END;
}

dmsg_t *dmsgDataPrepare2(msgType_t msg_type, subMsgType_t sub_msg_type, uint32_t msg_code, int trailing_space) {
    dmsg_t *msg = (dmsg_t *)calloc (1, sizeof (dmsg_t) + trailing_space);
    msg->msgId = 0;
    msg->msgType = msg_type;
    msg->subMsgType = sub_msg_type;
    msg->msgCode = msg_code;
    msg->tlvBufferSize = trailing_space;
    return msg;
}