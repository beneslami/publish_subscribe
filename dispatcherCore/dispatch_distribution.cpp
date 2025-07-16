#include <unordered_map>
#include <pthread.h>
#include <iostream>
#include <vector>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include "dispatchDB.h"
#include "pubsub.h"
#include "../common/comm_type.h"
#include "../common/dmsgOp.h"

typedef struct vectorData_ {
    dmsg_t *dmsg;
    std::shared_ptr<subscriberDBentry_t> sub_entry;
}vectorData_t;

class DispatcherQueue {
    private:
        std::vector<vectorData_t*> _distQueue[DMSG_PR_MAX];
        pthread_mutex_t _distQueueLock;
        pthread_cond_t _distQueueCond;
        void releaseDistributionQueue() {
            for(int i = 0; i < DMSG_PR_MAX; ++i) {
                for(auto vdata:_distQueue[i]) {
                    dmsgDereference(vdata->dmsg);
                    vdata->sub_entry = nullptr;
                    free(vdata);
                }
                _distQueue[i].clear();
            }
        }
    public:
        DispatcherQueue() {
            pthread_mutex_init(&_distQueueLock, NULL);
            pthread_cond_init(&_distQueueCond, NULL);
        }

        ~DispatcherQueue() {
            pthread_mutex_destroy(&_distQueueLock);
            pthread_cond_destroy(&_distQueueCond);
            releaseDistributionQueue();
        }

        void Enqueue(vectorData_t *vdata){
            pthread_mutex_lock(&_distQueueLock);
            _distQueue[vdata->dmsg->priority].push_back(vdata);
            pthread_cond_signal(&_distQueueCond);
            pthread_mutex_unlock(&_distQueueLock);
        }

        vectorData_t *Dequeue() {
            vectorData_t *vdata = NULL;
            pthread_mutex_lock(&_distQueueLock);
            while(1) {
                for(int i = 0; i < DMSG_PR_MAX; ++i) {
                    if(!_distQueue[i].empty()) {
                        vdata = _distQueue[i].front();
                        _distQueue[i].erase(_distQueue[i].begin());
                        break;
                    }
                }
                if(vdata) break;
                pthread_cond_wait(&_distQueueCond, &_distQueueLock);
            }
            pthread_mutex_unlock(&_distQueueLock);
            return vdata;
        }
};

#define DISPATCHER_DIST_QUEUES_MAX 2
static class DispatcherQueue *globalListQueue[DISPATCHER_DIST_QUEUES_MAX] = {0};

static void dispatcherDispatch(std::shared_ptr<subscriberDBentry_t>SubEntry, dmsg_t *dmsg) {
    std::cout << "Dispatcher : Dispatching message to subscriber\n";
    dmsgDebugPrint(dmsg);
    if (SubEntry->ipcType == IPC_TYPE_NONE) {
        printf ("Dispatcher : Error : Subscriber [%s, %u] IPC Channel Not Set\n", SubEntry->subName, SubEntry->subscriberId);
        return;
    }
    switch (SubEntry->ipcType) {
        case IPC_TYPE_NETSKT: {
            printf ("Dispatcher : Dispatching message to subscriber [%s, %u] over NETSKT\n", SubEntry->subName, SubEntry->subscriberId);
            uint32_t ip_addr = SubEntry->ipcStruct.netskt.ipAddr;
            uint16_t port = SubEntry->ipcStruct.netskt.port;
            uint8_t transport_type = SubEntry->ipcStruct.netskt.transportType;
            if (transport_type != IPPROTO_UDP) {
                printf ("Dispatcher Error : Unsupported Transport Type %u\n", transport_type);
                return;
            }
            struct sockaddr_in server_addr;
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(port);
            server_addr.sin_addr.s_addr = htonl(ip_addr);
            if (SubEntry->ipcStruct.netskt.sockFd > 0) {
                int rc = sendto(SubEntry->ipcStruct.netskt.sockFd, (char *)dmsg, sizeof(*dmsg) + dmsg->tlvBufferSize, 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
                if (rc < 0) {
                    printf ("Dispatcher : Error : Send Failed, errno = %d\n", errno);
                }
                break;
            }
            int sock_fd = socket (AF_INET, SOCK_DGRAM, transport_type);
            if (sock_fd < 0) {
                printf ("Dispatcher Error : Socket Creation Failed\n");
                return;
            }
            SubEntry->ipcStruct.netskt.sockFd = sock_fd;
            int rc = sendto (sock_fd, (char *)dmsg, sizeof(*dmsg) + dmsg->tlvBufferSize, 0, (struct sockaddr *)&server_addr, sizeof (struct sockaddr));
            if (rc < 0) {
                printf ("Dispatcher : Error : Send Failed, errno = %d\n", errno);
            }
        }
        break;
        case IPC_TYPE_MSGQ: {
            printf("Dispatcher : Dispatching message to subscriber [%s, %u] over MQUEUE\n", SubEntry->subName, SubEntry->subscriberId);
            break;
        }        
        case IPC_TYPE_UXSKT: {
            printf("Dispatcher : Dispatching message to subscriber [%s, %u] over UXSKT\n", SubEntry->subName, SubEntry->subscriberId);
            break;
        }        
        case IPC_TYPE_SHM: {
            printf("Dispatcher : Dispatching message to subscriber [%s, %u] over SHM\n", SubEntry->subName, SubEntry->subscriberId);
            break;
        }
        case IPC_TYPE_CBK: {
            printf("Dispatcher : Dispatching message to subscriber [%s, %u] over CBK\n", SubEntry->subName, SubEntry->subscriberId);
            if (SubEntry->ipcStruct.cbk.cbk) {
                SubEntry->ipcStruct.cbk.cbk (dmsg);
            }
            break;
        }
        default:
            std::cout << "Dispatcher Error : Unknown IPC Type " << SubEntry->ipcType << "\n";
    }

}

static void *dispatcherListenDistributionQueue(void *arg) {
    vectorData_t *vdata; 
    DispatcherQueue *dist_queue = static_cast<DispatcherQueue*>(arg);
    while(vdata = dist_queue->Dequeue()) { // will be blocked in case there is no item inside the queue
        dispatcherDispatch(vdata->sub_entry, vdata->dmsg);
        dmsgDereference(vdata->dmsg);
        vdata->sub_entry = nullptr;
        free(vdata);
    }
    return NULL;
}

void dispatcherForkDistributionThreads() {
    pthread_t *thread;
    for(int i = 0; i < DISPATCHER_DIST_QUEUES_MAX; ++i) {
        if(globalListQueue[i] == NULL) {
            globalListQueue[i] = new DispatcherQueue();
            thread = (pthread_t*)calloc(1, sizeof(pthread_t));
            pthread_create(thread, NULL, dispatcherListenDistributionQueue, (void*)globalListQueue[i]);
        }
    }      
}

static void dispatcherEnqueueDistributionQueue(dmsg_t *msg, std::shared_ptr<subscriberDBentry_t>sub_entry) {
    static int queue_index = 0;
    if(queue_index >= DISPATCHER_DIST_QUEUES_MAX) {
        queue_index = 0;
    }
    vectorData_t *vdata = (vectorData_t*)calloc(1, sizeof(vectorData_t));
    vdata->dmsg = msg;
    dmsgReference(msg);
    vdata->sub_entry = sub_entry;
    globalListQueue[queue_index]->Enqueue(vdata);
    queue_index++;
    std::cout << "Dispatcher: [dmsg, Subscriber: " << sub_entry->subName << "] Enqueued in Distribution Queue\n";
}

void dispatcherAcceptPubMsgForDistributionToSubscribers(dmsg_t *msg) {
    msg->msgType = DISPATCH_TO_SUB;
    pubSubDBentry_t *entry = pubSubDbGet(msg->msgCode);
    if(!entry) {
        return;
    }
    for(auto sub_entry: entry->subscribers) {
        dispatcherEnqueueDistributionQueue(msg, sub_entry);
    }
}