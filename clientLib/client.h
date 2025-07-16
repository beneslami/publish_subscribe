#pragma once 


#include "../common/comm_type.h"
#include "../common/ipc_struct.h"
#include "../common/dmsgOp.h"


class Client {
    private:
        ipcStruct_t *_ipcStruct;
        int _sockfd;
    protected:
        Client(ipcStruct_t *ipc_struct);
        virtual ~Client();
    public:
        int pubSubDispatchMsg(dmsg_t *dmsg);
        void dispatcherRegister(const char *entitiy_name, msgType_t msg_type);
        void dispatcherUnregister(uint32_t pub_id, msgType_t msg_type);
        int receiveFrom(char *arg, size_t size);
};

class PublisherClient : public Client {
    public:
        PublisherClient(ipcStruct_t * ipc_struct);
        ~PublisherClient();
        void publisherPublish(uint32_t pub_id, uint32_t msg_id);
        void publisherUnPublish(uint32_t pub_id, uint32_t msg_id);
};

class SubscriberClient : public Client {
    public:
        SubscriberClient(ipcStruct_t * ipc_struct);
        ~SubscriberClient();
        void subscriberSubscribe(uint32_t pub_id, uint32_t msg_id);
        void subscriberUnSubscribe(uint32_t pub_id, uint32_t msg_id);
        void subscriberSubscribeIpcChannel(uint32_t sub_id, ipcType_t ipc_type, ipcStruct_t *ipc_struct);
};



