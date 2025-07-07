#include "../common/comm_type.h"

void dispatcherRegister(int sock_fd, const char *entitiy_name, msgType_t msg_type);
void dispatcherUnregister(int sock_fd, uint32_t pub_id, msgType_t msg_type);
int pubSubDispatchMsg(int sock_fd, dmsg_t *dmsg);

/* below two APIs update pubDB only */
void publisherPublish(int sock_fd, uint32_t pub_id, uint32_t msg_id);
void publisherUnPublish(int sock_fd, uint32_t pub_id, uint32_t msg_id);

/* below two APIs update subDB and pubSubDB */
void subscriberSubscribe(int sock_fd, uint32_t pub_id, uint32_t msg_id);
void subscriberUnSubscribe(int sock_fd, uint32_t pub_id, uint32_t msg_id);