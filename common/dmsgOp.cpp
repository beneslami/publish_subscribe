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