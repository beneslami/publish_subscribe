#include "../common/comm_type.h"

void dispatcherRegister(int sock_fd, const char *entitiy_name, msgType_t msg_type);
void dispatcherUnregister(int sock_fd, uint32_t pub_id, msgType_t msg_type);
int pubSubDispatchMsg(int sock_fd, dmsg_t *dmsg);