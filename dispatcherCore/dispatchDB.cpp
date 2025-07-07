#include "pubsub.h"
#include "../common/dmsgOp.h"
#include "../common/comm_type.h"
#include "../common/ipc_struct.h"
#include "dispatchDB.h"
#include <unordered_map>
#include <assert.h>
#include <iostream>
#include <stdio.h>
#include <unordered_map>
#include <stdexcept>
#include <assert.h>
#include <algorithm>
#include <arpa/inet.h>
#include <stddef.h>
#include "../Libs/tlv.h"

std::unordered_map<uint32_t, publisherDBentry_t*> pubDB;                    // <msg_id, publisher instance>
std::unordered_map<uint32_t, std::shared_ptr<subscriberDBentry_t>>subDB;    // <msg_id, subscriber database entry>
std::unordered_map<uint32_t, pubSubDBentry_t*>pubSubDB;                     // <msg_id, list of subscribers subscribed to this message>

/* Publisher DB operations begin */
publisherDBentry_t * publisherDbCreate(uint32_t pub_id, char * pub_name) {
    if(pubDB.find(pub_id) != pubDB.end()){
        throw std::runtime_error("Publisher already exists");
        return NULL;
    }
    publisherDBentry_t *pub_entry = new publisherDBentry_t;
    if(pub_entry == NULL) {
        throw std::runtime_error("Memory allocation failed for creating Publisher database");
    }
    pub_entry->publisherId = pub_id;
    strncpy(pub_entry->pubName, pub_name, 64);
    pubDB[pub_id] = pub_entry;
    return pub_entry;
}

void publisherDbDelete(uint32_t pub_id) {
    auto item = pubDB.find(pub_id);
    if(item == pubDB.end()) {
        throw std::runtime_error("There is no such a publisher");
    }
    delete item->second;
    pubDB.erase(pub_id);
}

bool publisherPublishMsg(uint32_t pub_id, uint32_t published_msg_id) {
    return pubDB.find(pub_id) != pubDB.end();
    
}

bool publisherUnpublishMsg(uint32_t pub_id, uint32_t published_msg_id) {
    return pubDB.find(pub_id) != pubDB.end();
}
/* Publisher DB operations end */

/* Subscriber DB Operations begin */
std::shared_ptr<subscriberDBentry_t> subscriberDbCreate(uint32_t sub_id, char *sub_name) {
    if(subDB.find(sub_id) != subDB.end()){
        throw std::runtime_error("Subscriber already exists");
        return NULL;
    }
    std::shared_ptr<subscriberDBentry_t> sub_entry = std::make_shared<subscriberDBentry_t>(subscriberDBentry_t{});
    if(sub_entry == NULL) {
        throw std::runtime_error("Memory allocation failed for creating Subscriber database");
    }
    sub_entry->subscriberId = sub_id;
    strncpy(sub_entry->subName, sub_name, 64);
    subDB[sub_id] = sub_entry;
    return sub_entry;
}

void subscriberDbDelete(uint32_t sub_id) {
    subDB.erase(sub_id);
}

bool subscriberSubscribeMsg(uint32_t sub_id, uint32_t msg_id) {

}
bool subscriberUnsubscribeMsg(uint32_t sub_id, uint32_t msg_id) {

}
/* Subscriber DB Operations end */

/* Operations on PUB-SUB DB begin */
pubSubDBentry_t * pubSubDbCreate(uint32_t msg_id, std::shared_ptr<subscriberDBentry_t> su) {

}

void pubSubDbDelete(uint32_t msg_id, uint32_t sub_id) {

}

pubSubDBentry_t * pubSubDbGet(uint32_t msg_id) {

}
/* Operations on PUB-SUB DB end */

void dispatcherDbDisplay() {
    std::cout << "Publisher DB\n";
    for (auto it = pubDB.begin(); it != pubDB.end(); ++it) {
        std::cout << "Publisher ID : " << it->second->publisherId << ", Publisher Name : " << it->second->pubName << std::endl;
    }

    std::cout << "Subscriber DB\n";
    for (auto it = subDB.begin(); it != subDB.end(); ++it) {
        std::cout << "Subscriber ID : " << it->second->subscriberId << ", Subscriber Name : " << it->second->subName << std::endl; 
    }

    std::cout << "Pub-Sub DB\n";
    for (auto it = pubSubDB.begin(); it != pubSubDB.end(); ++it) {
        std::cout << "Message ID : " << it->second->publishMsgCode << std::endl;
        for (auto& sub : it->second->subscribers) {
            std::cout << "Subscriber ID : " << sub->subscriberId << std::endl;
        }
    }
}

bool dispatcherProcessSubscriberIpcSubscription(uint32_t sub_id, dmsg_t *dmsg) {
    assert(dmsg->msgType == SUB_TO_DISPATCH);
    assert(dmsg->subMsgType == SUB_MSG_IPC_CHANNEL_ADD);

    auto it = subDB.find(sub_id);
    if (it == subDB.end()) {
        printf("%s() : Error : Subscriber with ID %u not found.\n", __FUNCTION__, sub_id);
        return false;
    }

    auto SubEntry = it->second.get();

    if (SubEntry->ipcType != IPC_TYPE_NONE) {
        printf("Coordinator : Error: Subscriber [%s,%u] IPC Channel Already Exists\n", SubEntry->subName, SubEntry->subscriberId);
        return false;
    }

    if (dmsg->tlvBufferSize == 0) {
        std::cout << "Coordinator : Error: Subscriber IPC Channel TLV Contains no IPC Data\n";
        return false;
    }

    char *tlv_buffer = (char *)dmsg->tlvBuffer;
    size_t tlv_buffer_size = dmsg->tlvBufferSize;
    uint8_t tlv_data_len = 0;
    uint8_t tlv_type;
    char *tlv_value;

    ITERATE_TLV_BEGIN(tlv_buffer, tlv_type, tlv_data_len, tlv_value, tlv_buffer_size) {
        switch (tlv_type) {
            case TLV_IPC_NET_UDP_SKT: {
                    uint32_t ip_addr = *(uint32_t *)(tlv_value);
                    ip_addr = htonl(ip_addr);
                    uint16_t port = *(uint16_t *)(tlv_value + 4);
                    port = htons(port);
                    printf("Coordinator : Subscriber [%s,%u] IPC Channel Add : IP Address %u, Port %u\n", SubEntry->subName, SubEntry->subscriberId, ip_addr, port);

                    SubEntry->ipcType = IPC_TYPE_NETSKT;
                    SubEntry->ipcStruct.netskt.ipAddr = ip_addr;
                    SubEntry->ipcStruct.netskt.port = port;
                    SubEntry->ipcStruct.netskt.transportType = IPPROTO_UDP;
            }
            break;
        }
    } ITERATE_TLV_END;
    return true;
}

void pubSubDbDeleteSubscriber(std::shared_ptr<pubSubDBentry_t> SubEntry) {
    /*for (auto it = pubSubDB.begin(); it != pubSubDB.end(); ++it) {
        pubSubDBentry_t*pubEntry = it->second;
        auto &subscribers = pubEntry->subscribers;
        subscribers.erase(std::remove(subscribers.begin(), subscribers.end(), SubEntry), subscribers.end());
        if (subscribers.empty()) {
            delete pubEntry;
            it = pubSubDB.erase(it);
        } else {
            ++it;
        }
    }*/
}