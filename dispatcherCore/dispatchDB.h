#include <stdint.h>
#include "pubsub.h"

/* Publisher DB operations */
publisherDBentry_t * publisherDbCreate(uint32_t pub_id, char * pub_name);
void publisherDbDelete(uint32_t pub_id);
bool publisherPublishMsg(uint32_t pub_id, uint32_t published_msg_id);
bool publisherUnpublishMsg(uint32_t pub_id, uint32_t published_msg_id);

/* Subscriber DB Operations */
typedef struct subscriberDBEntry_ subscriberDBentry_t;
std::shared_ptr<subscriberDBentry_t> subscriberDbCreate(uint32_t sub_id, char *sub_name);
void subscriberDbDelete(uint32_t sub_id);
bool subscriberSubscribeMsg(uint32_t sub_id, uint32_t msg_id);
bool subscriberUnsubscribeMsg(uint32_t sub_id, uint32_t msg_id);

/* Operations on PUB-SUB DB */
typedef struct pubSubDbEntry_ pubSubDBentry_t;
pubSubDBentry_t * pubSubDbCreate(uint32_t msg_id, std::shared_ptr<subscriberDBentry_t> su);
void pubSubDbDelete(uint32_t msg_id, uint32_t sub_id);
pubSubDBentry_t * pubSubDbGet(uint32_t msg_id);
void pubSubDbDeleteSubscriber(std::shared_ptr<pubSubDBentry_t> SubEntry);

void dispatcherDbDisplay();