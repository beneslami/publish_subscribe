#ifndef __CMSG_OP__
#define __CMSG_OP__

#include "comm_type.h"

void dmsgDebugPrint(dmsg_t *dmsg);
dmsg_t *dmsgDataPrepare2(msgType_t msg_type, subMsgType_t sub_msg_type, uint32_t msg_code, int trailing_space);

#endif